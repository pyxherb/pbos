$ErrorActionPreference = "Stop"

$pbosTriplet = $env:PBOS_TRIPLET

if (-not $pbosTriplet) {
	Write-Output "LIMINE_PATH not set"
	Exit -1
}

$CC=$(where.exe clang)
$CXX=$(where.exe clang++)
$ASM=$(where.exe clang)
$LD=$(where.exe ld.lld)

echo "CC=$CC"
echo "CXX=$CXX"
echo "ASM=$ASM"
echo "LD=$LD"

for ($i = 0; $i -lt $args.Count; $i++) {
	switch ($args[$i]) {
		"--clean-previous" {
			Remove-Item build -Recurse -Force
		}
		Default {
			Write-Warning "Invalid argument ``$($args[$i])'"
		}
	}
}

cmake `
--no-warn-unused-cli `
-DCMAKE_BUILD_TYPE:STRING=Debug `
--toolchain ./cmake/Platform/$pbosTriplet-pbkim.cmake `
-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE `
-G "Ninja" `
-S . -B build
