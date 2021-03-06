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
 * Copyright (C) Cisco Systems Inc. 2000-2002.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */

/* 
 * Notes:
 *  - file formatted with tabstops == 4 spaces 
 */

#include <mp4av_common.h>

bool MP4AV_RfcIsmaConcatenator(
	MP4FileHandle mp4File, 
	MP4TrackId mediaTrackId, 
	MP4TrackId hintTrackId,
	u_int8_t samplesThisHint, 
	MP4SampleId* pSampleIds, 
	MP4Duration hintDuration,
	u_int16_t maxPayloadSize)
{
  // handle degenerate case
  if (samplesThisHint == 0) {
    return true;
  }

  u_int8_t auPayloadHdrSize;

  // LATER would be more efficient if this were a parameter
  u_int8_t mpeg4AudioType =
    MP4GetTrackAudioMpeg4Type(mp4File, mediaTrackId);

  if (mpeg4AudioType == MP4_MPEG4_CELP_AUDIO_TYPE) {
    auPayloadHdrSize = 1;
  } else {
    auPayloadHdrSize = 2;
  }

  // construct the new hint
  if (MP4AddRtpHint(mp4File, hintTrackId) == false ||
      MP4AddRtpPacket(mp4File, hintTrackId, true) == false) return false;

  u_int8_t payloadHeader[2];

  u_int16_t numHdrBits = samplesThisHint * auPayloadHdrSize * 8;
  payloadHeader[0] = numHdrBits >> 8;
  payloadHeader[1] = numHdrBits & 0xFF;

  if (MP4AddRtpImmediateData(mp4File, hintTrackId,
			     (u_int8_t*)&payloadHeader, 
			     sizeof(payloadHeader)) == false) return false;

  u_int8_t i;

  // first the headers
  for (i = 0; i < samplesThisHint; i++) {
    MP4SampleId sampleId = pSampleIds[i];

    u_int32_t sampleSize = 
      MP4GetSampleSize(mp4File, mediaTrackId, sampleId);

    if (auPayloadHdrSize == 1) {
      // AU payload header is 6 bits of size
      // follow by 2 bits of the difference between sampleId's - 1
      payloadHeader[0] = sampleSize << 2;

    } else { // auPayloadHdrSize == 2
      // AU payload header is 13 bits of size
      // follow by 3 bits of the difference between sampleId's - 1
      payloadHeader[0] = sampleSize >> 5;
      payloadHeader[1] = (sampleSize & 0x1F) << 3;
    }

    if (i > 0) {
      payloadHeader[auPayloadHdrSize - 1] 
	|= ((sampleId - pSampleIds[i-1]) - 1); 
    }
#if 0
    printf("sample %u size %u %02x %02x prev sample %d\n", 
	   sampleId, sampleSize, payloadHeader[0],
	   payloadHeader[1], pSampleIds[i-1]);
#endif

    if (MP4AddRtpImmediateData(mp4File, hintTrackId,
			       (u_int8_t*)&payloadHeader, 
			       auPayloadHdrSize) == false) 
      return false;
  }

  // then the samples
  for (i = 0; i < samplesThisHint; i++) {
    MP4SampleId sampleId = pSampleIds[i];

    u_int32_t sampleSize = 
      MP4GetSampleSize(mp4File, mediaTrackId, sampleId);

    if (MP4AddRtpSampleData(mp4File, hintTrackId, 
			    sampleId, 0, sampleSize) == false) return false;
  }

  // write the hint
  return MP4WriteRtpHint(mp4File, hintTrackId, hintDuration);
}

bool MP4AV_RfcIsmaFragmenter(
	MP4FileHandle mp4File, 
	MP4TrackId mediaTrackId, 
	MP4TrackId hintTrackId,
	MP4SampleId sampleId, 
	u_int32_t sampleSize, 
	MP4Duration sampleDuration,
	u_int16_t maxPayloadSize)
{
  if (MP4AddRtpHint(mp4File, hintTrackId) == false ||
      MP4AddRtpPacket(mp4File, hintTrackId, false) == false) 
    return false;

  // Note: CELP is never fragmented
  // so we assume the two byte AAC-hbr payload header
  u_int8_t payloadHeader[4];
  payloadHeader[0] = 0;
  payloadHeader[1] = 16;
  payloadHeader[2] = sampleSize >> 5;
  payloadHeader[3] = (sampleSize & 0x1F) << 3;

  if (MP4AddRtpImmediateData(mp4File, hintTrackId,
			     (u_int8_t*)&payloadHeader, 
			     sizeof(payloadHeader)) == false) 
    return false;

  u_int16_t sampleOffset = 0;
  u_int16_t fragLength = maxPayloadSize - 4;

  do {
    if (MP4AddRtpSampleData(mp4File, hintTrackId,
			    sampleId, 
			    sampleOffset, 
			    fragLength) == false) return false;

    sampleOffset += fragLength;

    if (sampleSize - sampleOffset > maxPayloadSize) {
      fragLength = maxPayloadSize; 
      if (MP4AddRtpPacket(mp4File, hintTrackId, false) == false) return false;
    } else {
      fragLength = sampleSize - sampleOffset; 
      if (fragLength) {
	if (MP4AddRtpPacket(mp4File, hintTrackId, true) == false) return false;
      }
    }
  } while (sampleOffset < sampleSize);

  return MP4WriteRtpHint(mp4File, hintTrackId, sampleDuration);
}

extern "C" bool MP4AV_RfcIsmaHinter(
	MP4FileHandle mp4File, 
	MP4TrackId mediaTrackId, 
	bool interleave,
	u_int16_t maxPayloadSize)
{
	// gather information, and check for validity

	u_int32_t numSamples =
		MP4GetTrackNumberOfSamples(mp4File, mediaTrackId);

	if (numSamples == 0) {
		return false;
	}

	u_int32_t timeScale =
		MP4GetTrackTimeScale(mp4File, mediaTrackId);

	if (timeScale == 0) {
		return false;
	}

	u_int8_t audioType =
		MP4GetTrackEsdsObjectTypeId(mp4File, mediaTrackId);

	if (audioType != MP4_MPEG4_AUDIO_TYPE
	  && !MP4_IS_AAC_AUDIO_TYPE(audioType)) {
		return false;
	}

	u_int8_t mpeg4AudioType =
		MP4GetTrackAudioMpeg4Type(mp4File, mediaTrackId);

	if (audioType == MP4_MPEG4_AUDIO_TYPE) {
		// check that track contains either MPEG-4 AAC or CELP
		if (!MP4_IS_MPEG4_AAC_AUDIO_TYPE(mpeg4AudioType) 
		  && mpeg4AudioType != MP4_MPEG4_CELP_AUDIO_TYPE) {
			return false;
		}
	}

	MP4Duration sampleDuration = 
		MP4AV_GetAudioSampleDuration(mp4File, mediaTrackId);

	if (sampleDuration == MP4_INVALID_DURATION) {
		return false;
	}

	/* get the ES configuration */
	u_int8_t* pConfig = NULL;
	u_int32_t configSize;
	uint8_t channels;

	if (MP4GetTrackESConfiguration(mp4File, mediaTrackId, 
				       &pConfig, &configSize) == false)
	  return false;

	if (!pConfig) {
		return false;
	}
     
	channels = MP4AV_AacConfigGetChannels(pConfig);

	/* convert ES Config into ASCII form */
	char* sConfig = 
		MP4BinaryToBase16(pConfig, configSize);

	free(pConfig);

	if (!sConfig) {
		return false;
	}

	/* create the appropriate SDP attribute */
	uint sdpBufLen = strlen(sConfig) + 256;
	char* sdpBuf = 
	  (char*)malloc(sdpBufLen);

	if (!sdpBuf) {
		free(sConfig);
		return false;
	}


	// now add the hint track
	MP4TrackId hintTrackId =
		MP4AddHintTrack(mp4File, mediaTrackId);

	if (hintTrackId == MP4_INVALID_TRACK_ID) {
		free(sConfig);
		free(sdpBuf);
		return false;
	}

	u_int8_t payloadNumber = MP4_SET_DYNAMIC_PAYLOAD;
	char buffer[10];
	if (channels != 1) {
	  snprintf(buffer, sizeof(buffer), "%u", channels);
	}
	if (MP4SetHintTrackRtpPayload(mp4File, hintTrackId, 
				      "mpeg4-generic", &payloadNumber, 0,
				      channels != 1 ? buffer : NULL) == false) {
	  MP4DeleteTrack(mp4File, hintTrackId);
	  free(sConfig);
	  free(sdpBuf);
	  return false;
	}

	MP4Duration maxLatency;
	bool OneByteHeader = false;
	if (mpeg4AudioType == MP4_MPEG4_CELP_AUDIO_TYPE) {
	  snprintf(sdpBuf, sdpBufLen,
			"a=fmtp:%u "
			"streamtype=5; profile-level-id=15; mode=CELP-vbr; config=%s; "
			"SizeLength=6; IndexLength=2; IndexDeltaLength=2; Profile=0;"
			"\015\012",
				payloadNumber,
				sConfig); 

		// 200 ms max latency for ISMA profile 1
		maxLatency = timeScale / 5;
		OneByteHeader = true;
	} else { // AAC
	  snprintf(sdpBuf, sdpBufLen,
			"a=fmtp:%u "
			"streamtype=5; profile-level-id=15; mode=AAC-hbr; config=%s; "
			"SizeLength=13; IndexLength=3; IndexDeltaLength=3;"
			"\015\012",
				payloadNumber,
				sConfig); 

		// 500 ms max latency for ISMA profile 1
		maxLatency = timeScale / 2;
	}

	/* add this to the track's sdp */
	bool val = MP4AppendHintTrackSdp(mp4File, hintTrackId, sdpBuf);

	free(sConfig);
	free(sdpBuf);
	if (val == false) {
	  MP4DeleteTrack(mp4File, hintTrackId);
	  return false;
	}

	u_int32_t samplesPerPacket = 0;
 
	if (interleave) {
		u_int32_t maxSampleSize =
			MP4GetTrackMaxSampleSize(mp4File, mediaTrackId);

		// compute how many maximum size samples would fit in a packet
		samplesPerPacket = 
			(maxPayloadSize - 2) / (maxSampleSize + 2);

		// can't interleave if this number is 0 or 1
		if (samplesPerPacket < 2) {
			interleave = false;
		}
	}

	bool rc;

	if (interleave) {
		u_int32_t samplesPerGroup = maxLatency / sampleDuration;
		u_int32_t stride;
		stride = samplesPerGroup / samplesPerPacket;

		if (OneByteHeader && stride > 3) stride = 3;
		if (!OneByteHeader && stride > 7) stride = 7;

#if 0
		printf("max latency %llu sampleDuration %llu spg %u spp %u strid %u\n",
		       maxLatency, sampleDuration, samplesPerGroup,
		       samplesPerPacket, stride);
#endif
		rc = MP4AV_AudioInterleaveHinter(
			mp4File, 
			mediaTrackId, 
			hintTrackId,
			sampleDuration, 
			stride,		// stride
			samplesPerPacket,						// bundle
			maxPayloadSize,
			MP4AV_RfcIsmaConcatenator);

	} else {
		rc = MP4AV_AudioConsecutiveHinter(
			mp4File, 
			mediaTrackId, 
			hintTrackId,
			sampleDuration, 
			2,										// perPacketHeaderSize
			2,										// perSampleHeaderSize
			maxLatency / sampleDuration,			// maxSamplesPerPacket
			maxPayloadSize,
			MP4GetSampleSize,
			MP4AV_RfcIsmaConcatenator,
			MP4AV_RfcIsmaFragmenter);
	}

	if (!rc) {
		MP4DeleteTrack(mp4File, hintTrackId);
		return false;
	}

	return true;
}

