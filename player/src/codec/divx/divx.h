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
 * divx.h - interface to divx library
 */
#ifndef __DIVX_H__
#define __DIVX_H__ 1
#include <systems.h>
#include "codec_plugin.h"
#include <fposrec/fposrec.h>

#define DIVX_STATE_VO_SEARCH 0
#define DIVX_STATE_NORMAL 1
#define DIVX_STATE_WAIT_I 2


typedef struct divx_codec_t {
  codec_data_t c;
  int m_nFrames;
  int m_decodeState;
  int m_dropFrame;
  uint64_t m_last_time;
  uint32_t m_dropped_b_frames;
  uint32_t m_num_wait_i;
  uint32_t m_num_wait_i_frames;
  uint32_t m_total_frames;
  // raw file support
  FILE *m_ifile;
  unsigned char *m_buffer;
  uint32_t m_buffer_size_max;
  uint32_t m_buffer_size;
  uint32_t m_buffer_on;
  uint32_t m_framecount;
  uint32_t m_frame_on;
  CFilePosRecorder *m_fpos;
} divx_codec_t;

#define m_vft c.v.video_vft
#define m_ifptr c.ifptr

void divx_clean_up(divx_codec_t *divx);

codec_data_t *divx_file_check(lib_message_func_t message,
			      const char *name, 
			      double *max,
			      char *desc[4]);
int divx_file_next_frame(codec_data_t *your_data,
			  unsigned char **buffer, 
			 uint64_t *ts);
void divx_file_used_for_frame(codec_data_t *your,uint32_t bytes);
int divx_file_seek_to(codec_data_t *you, uint64_t ts);
int divx_file_eof (codec_data_t *ifptr);
#endif
/* end file divx.h */
