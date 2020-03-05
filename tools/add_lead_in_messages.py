#!/usr/bin/python

# Adds a lead-in message to each mp3 file of a directory storing the result in another directory.
# So - when played e.g. on a TonUINO - you first will hear the title of the track, then the track itself.


import argparse, base64, json, os, re, subprocess, sys, text_to_speech


argFormatter = lambda prog: argparse.RawDescriptionHelpFormatter(prog, max_help_position=27, width=100)
argparser = text_to_speech.PatchedArgumentParser(
    description=
        'Adds a lead-in message to each mp3 file of a directory storing the result in another directory.\n' +
        'So - when played e.g. on a TonUINO - you first will hear the title of the track, then the track itself.\n\n' +
        text_to_speech.textToSpeechDescription,
    usage='%(prog)s -i my/source/dir -o my/output/dir [optional arguments...]',
    formatter_class=argFormatter)
argparser.add_argument('-i', '--input', type=str, required=True, help='The input directory or mp3 file to process (input won\'t be changed)')
argparser.add_argument('-o', '--output', type=str, required=True, help='The output directory where to write the mp3 files (will be created if not existing)')
text_to_speech.addArgumentsToArgparser(argparser)
argparser.add_argument('--file-regex', type=str, default=None, help="The regular expression to use for parsing the mp3 file name. If missing the whole file name except a leading number will be used as track title.")
argparser.add_argument('--title-pattern', type=str, default=None, help="The pattern to use as track title. May contain groups of `--file-regex`, e.g. '\\1'")
argparser.add_argument('--add-numbering', action='store_true', help='Whether to add a three-digit number to the mp3 files (suitable for DFPlayer Mini)')
argparser.add_argument('--dry-run', action='store_true', help='Dry run: Only prints what the script would do, without actually creating files')
args = argparser.parse_args()

text_to_speech.checkArgs(argparser, args)

fileRegex = re.compile(args.file_regex if args.file_regex is not None else '\\d*(.*)')
titlePattern = args.title_pattern if args.title_pattern is not None else '\\1'

mp3FileIndex = 0


def fail(msg):
    print('ERROR: ' + msg)
    sys.exit(1)


def addLeadInMessage(inputPath, outputPath):
    global mp3FileIndex

    if not os.path.exists(inputPath):
        fail('Input does not exist: ' + os.path.abspath(inputPath))

    if os.path.isdir(inputPath):
        if os.path.exists(outputPath):
            if not os.path.isdir(outputPath):
                fail('Input is a directory, but output isn\'t: ' + os.path.abspath(outputPath))
        elif not args.dry_run:
            os.mkdir(outputPath)

        mp3FileIndex = 0
        for child in sorted(os.listdir(inputPath)):
            addLeadInMessage(os.path.join(inputPath, child), os.path.join(outputPath, child))

        return

    inputFileNameSplit = os.path.splitext(os.path.basename(inputPath))
    inputFileName = inputFileNameSplit[0]
    inputFileExt = inputFileNameSplit[1].lower()

    if inputFileExt != '.mp3':
        print('Ignoring {} (no mp3 file)'.format(os.path.abspath(inputPath)))
        return

    if args.add_numbering:
        outputPathSplit = os.path.split(outputPath)
        outputPath = os.path.join(outputPathSplit[0], '{:0>3}_{}'.format(mp3FileIndex + 1, outputPathSplit[1]))
        mp3FileIndex += 1

    if os.path.isfile(outputPath):
        print('Skipping {} (file already exists)'.format(os.path.abspath(outputPath)))
        return

    text = re.sub(fileRegex, titlePattern, inputFileName).replace('_', ' ').strip()
    print('Adding lead-in "{}" to {}'.format(text, os.path.abspath(outputPath)))

    if not args.dry_run:
        tempLeadInFile = 'temp-lead-in.mp3'
        tempLeadInFileAdjusted = 'temp-lead-in_adjusted.mp3'
        text_to_speech.textToSpeechUsingArgs(text=text, targetFile=tempLeadInFile, args=args)

        # Adjust sample rate and mono/stereo
        print('Detecting sample rate and channels')
        detectionInfo = detectAudioData(inputPath)
        if detectionInfo is None:
            # We can't adjust
            print('Detecting sample rate and channels failed -> Skipping adjustment')
            tempLeadInFileAdjusted = tempLeadInFile
        else:
            print('Adjust sample rate to {} and channels to {}'.format(detectionInfo['sampleRate'], detectionInfo['channels']))
            subprocess.call([ 'ffmpeg', '-i', tempLeadInFile, '-vn', '-ar', detectionInfo['sampleRate'], '-ac', detectionInfo['channels'], tempLeadInFileAdjusted ])

        print('Concat')
        subprocess.call([ 'ffmpeg', '-i', 'concat:{}|{}'.format(tempLeadInFileAdjusted, inputPath), '-acodec', 'copy', outputPath, '-map_metadata', '0:1' ])

        os.remove(tempLeadInFile)
        os.remove(tempLeadInFileAdjusted)
        print('\n')


def detectAudioData(mp3File):
    try:
        output = subprocess.check_output([ 'ffmpeg', '-i', mp3File, '-hide_banner' ], stderr=subprocess.STDOUT)
    except Exception, e:
        output = str(e.output)

    match = re.match('.*Stream #\\d+:\\d+: Audio: mp3, (\\d+) Hz, (mono|stereo), .*', output, re.S)
    if match:
        return {
            'sampleRate': match.group(1),
            'channels': '2' if match.group(2) == 'stereo' else '1'
        }
    else:
        return None


if not os.path.exists(args.output) and not args.dry_run:
    outputParent = os.path.dirname(os.path.abspath(args.output))
    if not os.path.isdir(outputParent):
        fail('Parent of output is no directory: ' + os.path.abspath(outputParent))

addLeadInMessage(args.input, args.output)
