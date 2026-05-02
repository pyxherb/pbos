$ErrorActionPreference = "Stop"

$liminePath = $env:LIMINE_PATH

if (-not $liminePath) {
	Write-Output "LIMINE_PATH not set"
	Exit -1
}

if (-not (Test-Path $liminePath)) {
	Write-Output "LIMINE_PATH `"$liminePath`" does not exist"
	Exit -1
}

$imgPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath("build/boot.vhdx")
$flagRemovePrevious = $false

# Scan for each arguments.
for ($i = 0; $i -lt $args.Count; $i++) {
	switch ($args[$i]) {
		"--remove-previous" {
			$flagRemovePrevious = $true
		}
		Default {
			Write-Warning "Invalid argument ``$($args[$i])'"
		}
	}
}

function ExecDiskpartScript([string]$content) {
	if (-not $dpScriptFile) {
		$dpScriptFile = New-TemporaryFile
	}
	Clear-Content $dpScriptFile
	Write-Output $content | Out-File -Encoding "ascii" $dpScriptFile.FullName
	diskpart /s $dpScriptFile.FullName
}

function CreateDiskImage {
	ExecDiskpartScript "`
create vdisk MAXIMUM=256 FILE=`"$($imgPath)`"`n`
select vdisk FILE=`"$($imgPath)`"`n`
attach vdisk`n`
convert gpt`n`
create partition EFI SIZE=64`n`
select partition 2`n`
format FS=FAT32 QUICK`n`
create partition PRIMARY SIZE=160 OFFSET=81920`n`
select partition 3`n`
format FS=FAT32 QUICK`n`
"
}

function InstallLimine {
	New-Item "Y:\EFI\" -ItemType dir
	New-Item "Y:\EFI\BOOT\" -ItemType dir

	Copy-Item "$liminePath\BOOTX64.EFI" "Y:\EFI\BOOT"
	Copy-Item "$liminePath\BOOTIA32.EFI" "Y:\EFI\BOOT"
}

function UpdateLimineConfig {
	Copy-Item test\bootimg-x86_64\config\limine.conf X:\
}

function CopySystemFiles {
	Copy-Item build\pbkern B:\sys\kernel\
	Copy-Item build\initcar B:\sys\boot\
}

function RemovePreviousDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
detach vdisk`n"

	Remove-Item $imgPath
}

function AttachDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
attach vdisk`n"
}

function MountDiskImageEFI {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
select partition 2`n`
assign letter=Y`n`
"
}

function MountDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
select partition 3`n`
assign letter=X`n`
"
}

function UnmountDiskImage {
	ExecDiskpartScript "`
select vdisk FILE=`"$($imgPath)`"`n`
detach vdisk`n"
}

if ($flagRemovePrevious) {
	if (Test-Path $imgPath) {
		RemovePreviousDiskImage
	}
}

if (Test-Path "X:") {
	Write-Output "Error: Drive letter X is already in use"
	Exit -1;
}

if (-not (Test-Path $imgPath)) {
	if (Test-Path "Y:") {
		Write-Output "Error: Drive letter Y is already in use"
		Exit -1;
	}
	CreateDiskImage
	MountDiskImageEFI
	InstallLimine
} else {
	AttachDiskImage
}

MountDiskImage
UpdateLimineConfig
# CopySystemFiles

UnmountDiskImage
