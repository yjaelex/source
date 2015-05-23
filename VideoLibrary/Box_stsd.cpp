/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is MPEG4IP.
 *
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001 - 2004.  All Rights Reserved.
 *
 * 3GPP features implementation is based on 3GPP's TS26.234-v5.60,
 * and was contributed by Ximpo Group Ltd.
 *
 * Portions created by Ximpo Group Ltd. are
 * Copyright (C) Ximpo Group Ltd. 2003, 2004.  All Rights Reserved.
 *
 * Contributor(s):
 *      Dave Mackie         dmackie@cisco.com
 *      Alix Marchandise-Franquet   alix@cisco.com
 *              Ximpo Group Ltd.                mp4v2@ximpo.com
 */

#include "AllMP4Box.h"





MP4StsdBox::MP4StsdBox(MP4FileClass &file)
        : MP4Box(file, "stsd")
{
    AddVersionAndFlags();

    MP4Integer32Property* pCount =
        new MP4Integer32Property(*this, "entryCount");
    pCount->SetReadOnly();
    AddProperty(pCount);

    ExpectChildBox("mp4a", Optional, Many);
    ExpectChildBox("enca", Optional, Many);
    ExpectChildBox("mp4s", Optional, Many);
    ExpectChildBox("mp4v", Optional, Many);
    ExpectChildBox("encv", Optional, Many);
    ExpectChildBox("rtp ", Optional, Many);
    ExpectChildBox("samr", Optional, Many); // For AMR-NB
    ExpectChildBox("sawb", Optional, Many); // For AMR-WB
    ExpectChildBox("s263", Optional, Many); // For H.263
    ExpectChildBox("avc1", Optional, Many);
    ExpectChildBox("alac", Optional, Many);
    ExpectChildBox("text", Optional, Many);
    ExpectChildBox("ac-3", Optional, Many);
}

void MP4StsdBox::Read()
{
    /* do the usual read */
    MP4Box::Read();

    // check that number of children == entryCount
    MP4Integer32Property* pCount =
        (MP4Integer32Property*)m_pProperties[2];

    if (m_pChildBoxs.Size() != pCount->GetValue()) {
        log.warningf("%s: \"%s\": stsd inconsistency with number of entries",
                     __FUNCTION__, GetFile().GetFilename().c_str() );

        /* fix it */
        pCount->SetReadOnly(false);
        pCount->SetValue(m_pChildBoxs.Size());
        pCount->SetReadOnly(true);
    }
}



}
} // namespace mp4v2::impl
