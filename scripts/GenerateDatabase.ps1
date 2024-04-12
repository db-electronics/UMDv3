Param(
    [string]$Path = "./Genesis"
)


function Crc32($Crc, $Data) {
    $Crc = $Crc -bxor $Data

    for ($i = 0; $i -lt 32; $i++) {
        if ($Crc -band 0x80000000) {
            $Crc = ($Crc -shl 1) -bxor 0x04C11DB7 # Polynomial used in STM32
        }
        else {
            $Crc = $Crc -shl 1
        }
    }

    return $Crc
}


$Files = Get-ChildItem -Path $Path -Recurse -File

foreach ($File in $Files) {
    
    $Data = Get-Content -Path $File.FullName -Raw -Encoding Byte

    # read all bytes from the file and calculate CRC32 on each dword
    $Crc = 0xFFFFFFFF
    for ($i = 0; $i -lt $Data.Length; $i += 4) {
        $Crc = Crc32 $Crc ([BitConverter]::ToUInt32($Data, $i))
    }

    # create a file with a name that is the 8 character hexadecimal representation of the CRC32
    $CrcString = "{0:X8}.txt" -f $Crc
    New-Item -Path "$CrcString" -ItemType File -Force

}
