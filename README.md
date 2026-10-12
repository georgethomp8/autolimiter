# AutoLimit


<img width="772" height="459" alt="Screenshot 2026-10-12 at 4 28 58 PM" src="https://github.com/user-attachments/assets/2bb83411-ce77-4990-a148-09e28d448641" />


AutoLimit is a limiter plugin inspired by limiters like FabFilter Pro-L and Waves L2. Its adaptive release is inspired by Waves’ ARC (Auto Release Control). The plugin measures the signal’s crest factor and uses it to blend between two release profiles. The design blends a compressor with clipper to handle transients better. 

Signal Path is as follows 

input gain → compressor (adaptive release) → 4x oversampled soft clipper → ceiling

The input signal enters a compressor with fixed threshold, ratio.  The UI Exposes Attack, Release and a Crest Factor Smoothing Control which sets how fast the release reacts to changes in the audio.

The adaptive release is a mix between concepts presented in the AES paper Parameter Automation in a Dynamic Range Compressor by Reiss Et al. and the dual envelopes running in parallel found in the waves L2.

The signal then goes into a soft clipper followed by a hard clipper to enforce the limiters ceiling. 

The clipper runs at 4x oversampling using half-band equiripple FIR filters for a linear phase response. 
