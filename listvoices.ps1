Add-Type -AssemblyName "System.Speech"
$speaker = new-object System.Speech.Synthesis.SpeechSynthesizer

write-host "Folgende Sprachsynthesizer sind auf dem System installiert:`r`n" -ForegroundColor Yellow -BackgroundColor Black
$cntr = 0;
$voices = $speaker.GetInstalledVoices() | ?{$_.Enabled} | %{$_.VoiceInfo}
$voices | %{$cntr++;write-host "[$cntr] ($($_.Name)) ($($_.Culture))" -ForegroundColor Green}
