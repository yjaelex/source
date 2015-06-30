/*******************************************************************************
*           Template No. SWF0011   Template Version No. 03.01.00
*
*                       COPYRIGHT 2008 MOTOROLA
*                       ALL RIGHTS RESERVED.
*                    MOTOROLA CONFIDENTIAL PROPRIETARY
*
********************************************************************************
*
*   FILE NAME       : 3g2_file_access_api.c
*   FUNCTION NAME(S): 
*                     Buf_ReadInit()
*                     Buf_Get3gpp2DataSize()
*                     Buf_Read3gpp2Data()
*                     Buf_Read3gpp2Seek()
*                     Buf_ReadClose()
*
*   ORIGINATOR     : Devang M. Gohel (goheldevang@motorola.com)
*
*   DATE OF ORIGIN : 12/04/2008
*
*---------------------------------- PURPOSE ------------------------------------
*   Defines 3gpp2 buffer APIs.
*
*   References:
*
*   Design document Path:   YTD
*   Design document Name:  YTD
*
*--------------------------------- REVISIONS -----------------------------------
* Date      Name                 Tracking #   Description
* --------  -------------------  ----------   ----------------------------------
* 12/04/08 Devang M. Gohel      N/A     Created File.  
*******************************************************************************/

#include "3g2_http_access_api.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>

#if 0
struct fcurl_data
{
    enum fcurl_type_e type;     /* type of handle */
    union {
        CURL *curl;
        FILE *file;
    } handle;                   /* handle */

    char *buffer;               /* buffer to store cached data*/
    int buffer_len;             /* currently allocated buffers length */
    int buffer_pos;             /* end of data in buffer*/
    int still_running;          /* Is background url fetch still in progress */
;


typedef struct fcurl_data URL_FILE;



/* exported functions */
URL_FILE *url_fopen(const char *url,const char *operation);
int url_fclose(URL_FILE *file);
int url_feof(URL_FILE *file);
size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
char * url_fgets(char *ptr, int size, URL_FILE *file);
void url_rewind(URL_FILE *file);

/* we use a global one for convenience */
CURLM *multi_handle;
#endif 


/* curl calls this routine to get more data */
int seek_callback(void *instream, curl_off_t offset, int origin)
{

    curl_off_t CurlOffset;
    URL_FILE *fp;

        
    fprintf(stderr," ++ callback Seek \n");
 
    fp->buffer = (URL_FILE *)instream;
       
    switch (origin)
    {
        case SEEK_SET:
        CurlOffset = offset;
        fprintf(stderr," ++ callback Seek set %d \n",offset);
        break;
        
        case SEEK_CUR:
        CurlOffset = fp->buffer_pos+offset;
                
        fprintf(stderr," ++ callback Seek cur  %d \n",CurlOffset );
        
        break;

        case SEEK_END:
        CurlOffset = fp->buffer_len - offset;
        fprintf(stderr," ++ callback Seek end  %d \n",CurlOffset );
        break;
     }
        
     if((CurlOffset < 0 ) || ( CurlOffset >  fp->buffer_len))
     return -1;
     
    fp->buffer_pos = CurlOffset;
        
    
    return 0;

}      

size_t read_callback( void *ptr, size_t size, size_t nmemb, void *data ){

  URL_FILE *url = (URL_FILE *)data;

  size_t nBytes = size * nmemb;
  size_t toReadBytes = 0;
  size_t remainBuffBytes = url->buffer_len - url->buffer_pos;

  if ( url->buffer_pos < url->buffer_len ){

    toReadBytes = ( nBytes < remainBuffBytes ) ? nBytes : remainBuffBytes;
    memcpy( ptr, (char*)url->buffer + url->buffer_pos, toReadBytes );

    /* Update buffer offset */
    url->buffer_pos += toReadBytes;
    return toReadBytes;

  }

  return 0;
} 

/* curl calls this routine to get more data */
static size_t
write_callback(char *buffer,
               size_t size,
               size_t nitems,
               void *userp)
{
    char *newbuff;
    int rembuff;

    URL_FILE *url = (URL_FILE *)userp;
    size *= nitems;

    rembuff=url->buffer_len - url->buffer_pos; /* remaining space in buffer */

    if(size > rembuff)
    {
        /* not enough space in buffer */
        newbuff=realloc(url->buffer,url->buffer_len + (size - rembuff));
        if(newbuff==NULL)
        {
            fprintf(stderr,"callback buffer grow failed\n");
            size=rembuff;
        }
        else
        {
            /* realloc suceeded increase buffer size*/
            url->buffer_len+=size - rembuff;
            url->buffer=newbuff;

            /*printf("Callback buffer grown to %d bytes\n",url->buffer_len);*/
        }
    }

    memcpy(&url->buffer[url->buffer_pos], buffer, size);
    url->buffer_pos += size;

    /*fprintf(stderr, "callback %d size bytes\n", size);*/

    return size;
}

/* use to attempt to fill the read buffer up to requested number of bytes */
static int
fill_buffer(URL_FILE *file,int want,int waittime)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;
    struct timeval timeout;
    int rc;
    CURLMcode cres;

    /* only attempt to fill buffer if transactions still running and buffer
     * doesnt exceed required size already
     */
    if((!file->still_running) || (file->buffer_pos > want))
        return 0;

    /* attempt to fill buffer */
    do
    {
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to fail on */
        timeout.tv_sec = 5*60; /* 1 minute */
        timeout.tv_usec = 0;

        /* get file descriptors from the transfers */
        cres=curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
		if(cres!=CURLM_OK)
	    { 
	        printf("curl_multi_fdset failed %d\n",cres);
	        return -1;
	    }		

        /* In a real-world program you OF COURSE check the return code of the
           function calls, *and* you make sure that maxfd is bigger than -1
           so that the call to select() below makes sense! */

        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

        switch(rc) {
        case -1:
            /* select error */
            break;

        case 0:
            break;

        default:
            /* timeout or readable/writable sockets */
            /* note we *could* be more efficient and not wait for
             * CURLM_CALL_MULTI_PERFORM to clear here and check it on re-entry
             * but that gets messy */
            while(curl_multi_perform(multi_handle, &file->still_running) ==
                  CURLM_CALL_MULTI_PERFORM);

            break;
        }
    } while(file->still_running && (file->buffer_pos < want));
    return 1;
}

/* use to remove want bytes from the front of a files buffer */
static int
use_buffer(URL_FILE *file,int want)
{
    /* sort out buffer */
    if((file->buffer_pos - want) <=0)
    {
        /* ditch buffer - write will recreate */
        if(file->buffer)
            free(file->buffer);

        file->buffer=NULL;
        file->buffer_pos=0;
        file->buffer_len=0;
    }
    else
    {
        /* move rest down make it available for later */
        memmove(file->buffer,
                &file->buffer[want],
                (file->buffer_pos - want));

        file->buffer_pos -= want;
    }
    return 0;
}



URL_FILE *
url_fopen(const char *url,const char *operation)
{
    /* this code could check for URLs or types in the 'url' and
       basicly use the real fopen() for standard files */

    URL_FILE *file;
    (void)operation;

    file = malloc(sizeof(URL_FILE));
    if(!file)
        return NULL;

    memset(file, 0, sizeof(URL_FILE));

    if((file->handle.file=fopen(url,operation)))
    {
        file->type = CFTYPE_FILE; /* marked as URL */
    }
    else
    {
        file->type = CFTYPE_CURL; /* marked as URL */
        file->handle.curl = curl_easy_init();

        curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
        curl_easy_setopt(file->handle.curl, CURLOPT_FILE, file);

       // curl_easy_setopt(file->handle.curl, CURLOPT_WRITEDATA, file);
       // curl_easy_setopt(file->handle.curl, CURLOPT_SEEKDATA, file);
        curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, write_callback);
       // curl_easy_setopt(file->handle.curl, CURLOPT_SEEKFUNCTION, seek_callback);


        if(!multi_handle)
            multi_handle = curl_multi_init();

        curl_multi_add_handle(multi_handle, file->handle.curl);

        /* lets start the fetch */
        while(curl_multi_perform(multi_handle, &file->still_running) ==
              CURLM_CALL_MULTI_PERFORM );

        if((file->buffer_pos == 0) && (!file->still_running))
        {
            /* if still_running is 0 now, we should return NULL */

            /* make sure the easy handle is not in the multi handle anymore */
            curl_multi_remove_handle(multi_handle, file->handle.curl);

            /* cleanup */
            curl_easy_cleanup(file->handle.curl);

            free(file);

            file = NULL;
        }
    }
    return file;
}

int
url_fclose(URL_FILE *file)
{
    int ret=0;/* default is good return */

    switch(file->type)
    {
    case CFTYPE_FILE:
        ret=fclose(file->handle.file); /* passthrough */
        break;

    case CFTYPE_CURL:
        /* make sure the easy handle is not in the multi handle anymore */
        curl_multi_remove_handle(multi_handle, file->handle.curl);

        /* cleanup */
        curl_easy_cleanup(file->handle.curl);
        break;

    default: /* unknown or supported type - oh dear */
        ret=EOF;
        errno=EBADF;
        break;

    }

    if(file->buffer)
        free(file->buffer);/* free any allocated buffer space */

    free(file);

    return ret;
}

int
url_feof(URL_FILE *file)
{
    int ret=0;

    switch(file->type)
    {
    case CFTYPE_FILE:
        ret=feof(file->handle.file);
        break;

    case CFTYPE_CURL:
        if((file->buffer_pos == 0) && (!file->still_running))
            ret = 1;
        break;
    default: /* unknown or supported type - oh dear */
        ret=-1;
        errno=EBADF;
        break;
    }
    return ret;
}

size_t
url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file)
{
    size_t want;

    switch(file->type)
    {
    case CFTYPE_FILE:
        want=fread(ptr,size,nmemb,file->handle.file);
        break;

    case CFTYPE_CURL:
        want = nmemb * size;

        fill_buffer(file,want,1);

        /* check if theres data in the buffer - if not fill_buffer()
         * either errored or EOF */
        if(!file->buffer_pos)
            return 0;

        /* ensure only available data is considered */
        if(file->buffer_pos < want)
            want = file->buffer_pos;

        /* xfer data to caller */
        memcpy(ptr, file->buffer, want);

        use_buffer(file,want);

        want = want / size;     /* number of items - nb correct op - checked
                                 * with glibc code*/

        /*printf("(fread) return %d bytes %d left\n", want,file->buffer_pos);*/
        break;

    default: /* unknown or supported type - oh dear */
        want=0;
        errno=EBADF;
        break;

    }
    return want;
}


char *
url_fgets(char *ptr, int size, URL_FILE *file)
{
    int want = size - 1;/* always need to leave room for zero termination */
    int loop;

    switch(file->type)
    {
    case CFTYPE_FILE:
        ptr = fgets(ptr,size,file->handle.file);
        break;

    case CFTYPE_CURL:
        fill_buffer(file,want,1);

        /* check if theres data in the buffer - if not fill either errored or
         * EOF */
        if(!file->buffer_pos)
            return NULL;

        /* ensure only available data is considered */
        if(file->buffer_pos < want)
            want = file->buffer_pos;

        /*buffer contains data */
        /* look for newline or eof */
        for(loop=0;loop < want;loop++)
        {
            if(file->buffer[loop] == '\n')
            {
                want=loop+1;/* include newline */
                break;
            }
        }

        /* xfer data to caller */
        memcpy(ptr, file->buffer, want);
        ptr[want]=0;/* allways null terminate */

        use_buffer(file,want);

        /*printf("(fgets) return %d bytes %d left\n", want,file->buffer_pos);*/
        break;

    default: /* unknown or supported type - oh dear */
        ptr=NULL;
        errno=EBADF;
        break;
    }

    return ptr;/*success */
}

void
url_rewind(URL_FILE *file)
{
    char * url;

    switch(file->type)
    {
    case CFTYPE_FILE:
	rewind(file->handle.file); /* passthrough */
	break;

    case CFTYPE_CURL:
	/* halt transaction */
	curl_multi_remove_handle(multi_handle, file->handle.curl);
	curl_multi_cleanup(multi_handle);

	curl_easy_getinfo(file->handle.curl, CURLINFO_EFFECTIVE_URL, &url );

	/* cleanup */
	curl_easy_cleanup(file->handle.curl);

	/* ditch buffer - write will recreate - resets stream pos*/
	if(file->buffer)
	    free(file->buffer);

	file->buffer=NULL;
	file->buffer_pos=0;
	file->buffer_len=0;

	file->handle.curl = curl_easy_init();

	curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
	curl_easy_setopt(file->handle.curl, CURLOPT_FILE, file);
	curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, write_callback);

	multi_handle = curl_multi_init();

	/* restart */
	curl_multi_add_handle(multi_handle, file->handle.curl);

	/* re-start the fetch! */
	while(curl_multi_perform(multi_handle, &file->still_running) ==
	      CURLM_CALL_MULTI_PERFORM );

	break;

    default: /* unknown or supported type - oh dear */
	break;

    }

}

int
url_fgetc(URL_FILE *file)
{
    unsigned char retc;
    int reti;
    reti = url_fread(&retc, 1, 1, file);
    if(reti!=1)
	return reti;

    return retc;
}

int 
url_fseek(URL_FILE *file, long offset, int whence)
{
    char * url;

    switch(file->type)
    {
    case CFTYPE_FILE:
	fseek(file->handle.file,offset,whence); /* passthrough */
	break;

    case CFTYPE_CURL:
          
        file->type = CFTYPE_CURL; /* marked as URL */
	file->handle.curl = curl_easy_init();

	curl_easy_setopt(file->handle.curl, CURLOPT_FILE, file);
        curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(file->handle.curl, CURLOPT_READFUNCTION, read_callback );
        curl_easy_setopt(file->handle.curl, CURLOPT_READDATA, file); 
        curl_easy_setopt(file->handle.curl, CURLOPT_SEEKFUNCTION, seek_callback);
        curl_easy_setopt(file->handle.curl, CURLOPT_SEEKDATA, NULL);

        curl_easy_setopt(file->handle.curl, CURLOPT_UPLOAD, 1L );
        curl_easy_perform(file->handle.curl); 
         
        curl_easy_setopt(file->handle.curl, CURLOPT_RESUME_FROM, 4 ); 
        curl_easy_perform(file->handle.curl); 

 
        if(!multi_handle)
            multi_handle = curl_multi_init();

        curl_multi_add_handle(multi_handle, file->handle.curl);

        /* lets start the fetch */
        while(curl_multi_perform(multi_handle, &file->still_running) ==
              CURLM_CALL_MULTI_PERFORM );
        if((file->buffer_pos == 0) && (!file->still_running))
        {
            /* if still_running is 0 now, we should return NULL */

            /* make sure the easy handle is not in the multi handle anymore */
            curl_multi_remove_handle(multi_handle, file->handle.curl);

            /* cleanup */
            curl_easy_cleanup(file->handle.curl);

            free(file);

            file = NULL;
        }

	break;

    default: /* unknown or supported type - oh dear */
	break;

    }
    return 0;
}   

#if 0
int
url_fseek(URL_FILE *stream, long offset, int whence)
{
    url_fread(&retc, 1, 1, file);
    return -1;
}
#endif











BUFHTTP3G2_ERR 
Buf_ReadHttpInit(void **ctx, const char *filename, unsigned char *modulekey)
{
  URL_FILE *ctx3gp ; 
  modulekey = NULL;

  ctx3gp = url_fopen(filename,"rb");
  if (ctx3gp == NULL)
  {
    return BUFHTTP3G2_ERR_OPEN;
  }

  *ctx = ctx3gp;
  return BUFHTTP3G2_ERR_OK;
}

BUFHTTP3G2_ERR Buf_GetHttp3gpp2DataSize(const void *ctx, unsigned long *size)
{
	URL_FILE *ctx3gp = ( URL_FILE *)ctx;
        char buffer[256];
	if(ctx3gp==NULL)
	{
		return BUFHTTP3G2_ERR_INVALIDPARAM;
	}

        while(!url_feof(ctx3gp)) 
        {
           url_fgets(buffer,sizeof(buffer),ctx3gp);
                   
        }
	*size = sizeof(buffer); //ctx3gp->buffer_len;
	return BUFHTTP3G2_ERR_OK;
}

BUFHTTP3G2_ERR 
Buf_ReadHttp3gpp2Data(const void *ctx, unsigned char *buff, 
                      unsigned long count, unsigned long *readbyte)
{
  URL_FILE *ctx3gp = ( URL_FILE *)ctx;
  if((ctx3gp==NULL)||(buff==NULL)||(count==0)) 
  {
    return BUFHTTP3G2_ERR_INVALIDPARAM;
  }

  url_fgets(buff,count,ctx3gp);

  return BUFHTTP3G2_ERR_OK;
}

BUFHTTP3G2_ERR 
Buf_ReadHttp3gpp2Seek(const void *ctx, unsigned long offset)
{
  URL_FILE *ctx3gp = (URL_FILE *)ctx;
  char buffer[256];

  if(ctx3gp==NULL)
  {
    return BUFHTTP3G2_ERR_INVALIDPARAM;
  }
  	
  url_rewind(ctx3gp); 

  if (url_fgets(buffer,offset,ctx3gp) < 0)
  {
      return BUFHTTP3G2_ERR_3G2SEEK;

  }   
 
  ctx3gp->buffer_pos += offset;
  
  return BUFHTTP3G2_ERR_OK;
}


BUFHTTP3G2_ERR Buf_ReadHttpClose(const void *ctx)
{
	URL_FILE *ctx3gp = (URL_FILE *)ctx; 
	if(ctx3gp==NULL)
	{
		return BUFHTTP3G2_ERR_INVALIDPARAM;
	}
	url_fclose(ctx3gp);
	return BUFHTTP3G2_ERR_OK;
}

