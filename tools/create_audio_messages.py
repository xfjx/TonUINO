#!/usr/bin/python

# Creates the audio messages needed by TonUINO.


import argparse, os, re, shutil, sys, text_to_speech


if __name__ == '__main__':
    argFormatter = lambda prog: argparse.RawDescriptionHelpFormatter(prog, max_help_position=30, width=100)
    argparser = text_to_speech.PatchedArgumentParser(
        description=
            'Creates the audio messages needed by TonUINO.\n\n' +
            text_to_speech.textToSpeechDescription,
        usage='%(prog)s [optional arguments...]',
        formatter_class=argFormatter)
    argparser.add_argument('-i', '--input', type=str, default='.', help='The directory where `audio_messages_*.txt` files are located. (default: current directory)')
    argparser.add_argument('-o', '--output', type=str, default='sd-card', help='The directory where to create the audio messages. (default: `sd-card`)')
    text_to_speech.addArgumentsToArgparser(argparser)
    argparser.add_argument('--skip-numbers', action='store_true', help='If set, no number messages will be generated (`0001.mp3` - `0255.mp3`)')
    argparser.add_argument('--only-new', action='store_true', help='If set, only new messages will be created.')
    args = argparser.parse_args()


    text_to_speech.checkArgs(argparser, args)

    audioMessagesFile = '{}/audio_messages_{}.txt'.format(args.input, args.lang)
    if not os.path.isfile(audioMessagesFile):
        print('Input file does not exist: ' + os.path.abspath(audioMessagesFile))
        exit(1)

    targetDir = args.output
    if os.path.isdir(targetDir):
        print("Directory `" + targetDir + "` already exists.")
    else:
        os.mkdir(targetDir)
        os.mkdir(targetDir + '/advert')
        os.mkdir(targetDir + '/mp3')


    if not args.skip_numbers:
        for i in range(1,256):
            targetFile1 = '{}/mp3/{:0>4}.mp3'.format(targetDir, i)
            targetFile2 = '{}/advert/{:0>4}.mp3'.format(targetDir, i)
            text_to_speech.textToSpeechUsingArgs(text='{}'.format(i), targetFile=targetFile1, args=args)
            shutil.copy(targetFile1, targetFile2)

    with open(audioMessagesFile) as f:
        lineRe = re.compile('^([^|]+)\\|(.*)$')
        for line in f:
            match = lineRe.match(line.strip())
            if match:
                fileName = match.group(1)
                if args.only_new and os.path.isfile(targetDir + "/" + fileName):
                    continue
                text = match.group(2)
                text_to_speech.textToSpeechUsingArgs(text=text, targetFile=targetDir + "/" + fileName, args=args)
