# Personal DLSS frame generation provenance

The personal branch adapts StreamlineIntegration.cpp/.h and camera/input conversion
concepts from brodrigz/xray-monolith release 0.6, tag 1.0.2 (bdefda0):
https://github.com/brodrigz/xray-monolith/tree/1.0.2
Upstream credits OGSR team and xrSimpAI. Those credits remain in adapted files.
The repository's GSC license remains supplied as License.txt. No frozen weapon-follow
or pivot changes were merged.

The bridge, its input transport and conservative UI protection are integrated for
Squared Away's Bodycam renderer. Item/camera UI, scope and vision effects suspend FG. Test 4 captures actor HUD widgets/glass separately.
The binary UI mask protects changed display pixels; it is not recovered translucent
UI alpha. This limitation needs visual testing.

Official NVIDIA Streamline 2.14.16 headers and licenses are retained under sdk/Streamline.
Production DLLs are downloaded from its official release; the SDK archive SHA256 is
32041e51db0cab709a271b72a352281b690c2abbd669a827809401a5dd23956b.
Each packaged DLL must have a valid NVIDIA Authenticode signature. Only the needed
production SL/DLSS-G/Reflex/PCL libraries are packaged. No development/watermarked DLLs.
This software contains source code provided by NVIDIA Corporation.

References:
https://github.com/NVIDIA-RTX/Streamline/releases/tag/v2.14.16
https://github.com/NVIDIA-RTX/Streamline/blob/v2.14.16/docs/ProgrammingGuideDLSS_G.md
https://github.com/NVIDIA-RTX/Streamline/blob/v2.14.16/docs/ProgrammingGuideReflex.md


Test 4 references (2026-10-18; no OptiScaler code copied): official optiscaler/OptiScaler master babf1d6c1450dba634dd2f39de00d87698ccbef4, latest release v0.9.4; wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass bundled commit 5019c979656225ec87e3331658f8a1f3ea478244, reference HEAD 1bd39091337cc07ba961c8e59ded57e21dc95b18, release v0.8.3. The installed/Full Pack dxgi.dll SHA256 CF9C59158477B84A414334819E07A961530FF1308C0BB9FD3849CEB2A276C01E matched exactly. External FG mode in that fork intentionally preserves NR/SR while bypassing Streamline/Reflex hooks. Official upstream has different feature coverage; no wholesale update is bundled.
https://github.com/optiscaler/OptiScaler
https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass
