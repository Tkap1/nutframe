mkdir normalized | Out-Null
$files = (Get-ChildItem *.wav)
foreach ($file in $files)
{
  $name = $file.Name
  write-host $name
  ffmpeg -loglevel error -i "$name" -ar 44100 -af loudnorm=I=-20 "normalized\$name"
}