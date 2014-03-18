# $Id: 201_codec_speex_16000.py 2063 2008-06-26 18:52:16Z nanang $
#
from inc_cfg import *

# Call with Speex/16000 codec
test_param = TestParam(
		"PESQ codec Speex WB (RX side uses snd dev)",
		[
			InstanceParam("UA1", "--max-calls=1 --clock-rate 16000 --add-codec speex/16000 --play-file wavs/input.16.wav --null-audio"),
			InstanceParam("UA2", "--max-calls=1 --clock-rate 16000 --add-codec speex/16000 --rec-file  wavs/tmp.16.wav   --auto-answer 200")
		]
		)

if (HAS_SND_DEV == 0):
	test_param.skip = True

pesq_threshold = 3.7
