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
 * Copyright (C) Cisco Systems Inc. 2000, 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *              Bill May        wmay@cisco.com
 */
/*
 * aa.h - class definition for AAC codec.
 */

#ifndef __AA_H__
#define __AA_H__ 1
#include "systems.h"
#include <faad/all.h>
#include <faad/bits.h>
#include "player_util.h"

#include "codec.h"
#include "audio.h"
//#define DUMP_OUTPUT_TO_FILE 1
class CAACodec : public CAudioCodecBase {
 public:
  CAACodec(CAudioSync *a,
	   COurInByteStream *pbytestrm,
	   format_list_t *media_desc,
	   audio_info_t *audio,
	   const unsigned char *userdata = NULL,
	   uint32_t userdata_size = 0);
  ~CAACodec();
  int decode(uint64_t rtptime, int fromrtp, unsigned char *buffer, uint32_t buflen);
  int skip_frame(uint64_t rtptime, unsigned char *buffer, uint32_t buflen);
  void skip_frame(void);
  void do_pause(void);
 private:
  faacDecHandle m_info;
  int m_object_type;
  int m_resync_with_header;
  int m_record_sync_time;
  uint64_t m_current_time;
  uint64_t m_last_rtp_ts;
  uint64_t m_msec_per_frame;
  uint32_t m_current_frame;
  SDL_AudioSpec m_obtained;
  int m_audio_inited;
  int m_faad_inited;
  int m_freq;  // frequency
  int m_chans; // channels
  int m_output_frame_size;
  unsigned char *m_temp_buff;
#if DUMP_OUTPUT_TO_FILE
  FILE *m_outfile;
#endif
};

#ifdef _WIN32
DEFINE_MESSAGE_MACRO(aa_message, "aac")
#else
#define aa_message(loglevel, fmt...) message(loglevel, "aa", fmt)
#endif
#endif
