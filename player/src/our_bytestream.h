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
 * our_bytestream.h - base class for access to various "getbyte" facilities
 * such as rtp, file, quicktime file, memory.
 *
 * virtual routines here will allow any type of byte stream to be read
 * by the codecs.
 */
#ifndef __OUR_BYTESTREAM_H__
#define __OUR_BYTESTREAM_H__ 1
#include <assert.h>
#include "systems.h"
#include "codec/codec.h"

class COurInByteStream
{
 public:
  COurInByteStream() {};
  COurInByteStream(const char *name) {
    m_name = name;
  };
  virtual ~COurInByteStream() {};
  virtual int eof(void) = 0;
  virtual unsigned char get(void) = 0;
  virtual unsigned char peek(void) = 0;
  virtual void bookmark(int bSet) = 0;
  virtual void reset(void) = 0;
  virtual int have_no_data (void) {return 0; };
  virtual uint64_t start_next_frame (unsigned char **buffer, uint32_t *buflen) = 0;
  virtual void used_bytes_for_frame(uint32_t bytes) { };
  virtual void get_more_bytes (unsigned char **buffer,
			       uint32_t *buflen,
			       uint32_t used,
			       int get) = 0;
  virtual int can_skip_frame (void) { return 0; };
  virtual int skip_next_frame (uint64_t *ts, int *hasSyncFrame, unsigned char **buffer, uint32_t *buflen) { assert(FALSE);return 0; };
  virtual double get_max_playtime (void) = 0;
  virtual void set_start_time(uint64_t start) { m_play_start_time = start; };
  virtual ssize_t read(unsigned char *buffer, size_t bytes) = 0;
  virtual ssize_t read(char *buffer, size_t bytes) = 0;
  virtual void set_codec (CCodecBase *codec) { m_codec = codec; };
  virtual const char *get_throw_error(int error) = 0;
  // A minor error is one where in video, you don't need to skip to the
  // next I frame.
  virtual int throw_error_minor(int error) = 0;
 protected:
  uint64_t m_play_start_time;
  CCodecBase *m_codec;
  const char *m_name;
};

#endif

