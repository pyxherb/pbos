$ErrorActionPreference = "Stop"

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
--toolchain cmake/platform/i386-pc-pbkim.cmake `
-DCMAKE_C_COMPILER:FILEPATH="$CC" `
-DCMAKE_CXX_COMPILER:FILEPATH="$CXX" `
-DCMAKE_ASM_COMPILER:FILEPATH="$ASM" `
-DCMAKE_LINKER:FILEPATH="$LD" `
-G "Ninja" `
-S . -B build
