param (
    [string]$csvfile = ((Get-Item -Path ".\").FullName + "\soundfiles.txt")
 )

$patharray = @()
$text2speech = @()

$ffmpegpath = ".'c:\FFMPEG\bin\ffmpeg.exe'"
$outputpath = (Get-Item -Path ".\").FullName + "\SD-Karte-Microsoft-Stimme"

# add the number 1..255
for ($i=1; $i -le 255; $i++){
    $patharray += ("mp3/" + $i.ToString("#0000") + ".mp3")
    $text2speech += $i.ToString("0")
	$patharray += ("advert/" + $i.ToString("#0000") + ".mp3")
    $text2speech += $i.ToString("0")
}

Import-Csv $csvfile -delimiter "|" -Header "path","text" |`
    ForEach-Object {
        $patharray += $_.path
        $text2speech += $_.text
    }

Add-Type -AssemblyName "System.Speech"
$synthFormat = New-Object -TypeName System.Speech.AudioFormat.SpeechAudioFormatInfo(32000, 16, 1); 
$speaker = new-object System.Speech.Synthesis.SpeechSynthesizer
$voice = "Microsoft Hedda Desktop"

Write-Host $voice

ForEach ($apath in $patharray) {
    write-host $apath
    write-host $text2speech[$patharray.IndexOf($apath)]

    if ($apath.StartsWith("#")) {
        continue
    }
    
    $outputfile = $outputpath + "\" + $apath + ".wav"
    $outputdir = Split-Path -Path $outputfile
    New-Item -ItemType Directory -Force -Path $outputdir

    # $speaker.SetOutputToDefaultAudioDevice()
    $speaker.SetOutputToWavefile($outputfile, $synthFormat)

    $speaker.SelectVoice($voice)
    $speaker.Speak($text2speech[$patharray.IndexOf($apath)])
    $speaker.SetOutputToDefaultAudioDevice()

    $mp3output = ((Get-Item $outputfile).DirectoryName + "\" + (Get-Item $outputfile).Basename)
    
    # and convert it
    $rate = '32k'
    #-i Input file path
    #-id3v2_version Force id3 version so windows can see id3 tags
    #-f Format is MP3
    #-ab Bit rate
    #-ar Frequency
    # Output file path
    #-y Overwrite the destination file without confirmation
    #-v error only error output
    $arguments = "-i `"$outputfile`" -id3v2_version 3 -f mp3 -ab $rate -ar 44100 `"$mp3output`" -y -v error"
    Invoke-Expression "$ffmpegpath $arguments"
    Write-Host "$outputfile converted to $mp3output"
    Remove-Item -Path $outputfile
    Write-Output ""
}    

        