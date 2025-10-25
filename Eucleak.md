# EUCLEAK  CVE-2024-45678

## 1. Explain the core vulnerability 
>exploited in the EUCLEAK attack and why it remained undetected for over a decade despite numerous Common Criteria (CC) evaluations.

Infineon Technologies delivers a cryptographic library which implements the ECDSA/Elliptic Curve Digital Signature Algorithm. That implementation in turn uses an implementation of the Extended Euclidean Algorithm (EEA) for modular inversion which is not constant in time. The cryptographic library is contained in the firmware of several security devices, e.g. the YubiKey 5 series. By measuring the emitted electromagnetic (EM) emissions of such devices while executing several signing operations using the same private key stored in the device, the researchers managed to re-establish that key.

The paper does not give an explicit statement why the vulnerability got unnoticed between its introduction in 2010 and its discovery in 2024. Apparently, the CC evaluations do not include the analysis of EM emissions of the certified devices, at least not in the manner in which NinjaLab conducted the research. Finding the side channel and exploiting it took NinjaLab roughly two years.  

EM analysis requires expensive sensor devices and technicians who are able to use them effectively. For an EM analysis, the package of a device must be opened, usually a complex task which risks destroying it altogether. Therefore it is likely that the CC evaluations only conduct EM analysis on packaged devices or not at all.

The easiest way to find the vulnerability in the CC evaluations would have been an analysis of the EEA source code (including the implemented side channel defense mechanisms), and the analysis of its timing properties. Apparently, the side channel and the weak masking were missed in the code review, and no (sufficient) timing analysis was done.

## 2. Outline the step-by-step attack chain
>described in the paper for cloning a YubiKey using EUCLEAK.

The attack is conducted in two phases. In the *online phase*, the device gets into the attackers hands without the owner noticing it. The attacker opens the package of the device, and records EM emissions by placing an EM probe close to the processor die while executing several ECDSA signing operations using the device and the key in it that shall be obtained. Thereafter, the device is repackaged -without leaving marks of the tampering- and returned to the owner. The paper author states that this phase usually lasts for an hour, most of the time is required for opening and repackaging the device.

The *offline phase*, includes the analysis of the collected EM patterns and pre-processing steps (partly done manually). Thereafter the traces of the EEA algorithms are extracted, from that extracted traces the leaked information is taken, and finally the key is restored by using the leaked information. That process takes roughly a day. If the whole process were automated, the author assumes that it can be done in an hour.

## 3. How did the researchers confirm
>that the leaked operation was indeed an Extended Euclidean Algorithm computation?

The researchers analyzed the EM patterns while executing a signature verification operation in which `k` and `N` were both known for the calculation of `k^(-1) mod N`. The cryptographic library did *not* mask `k` in for signature verification, presumably because `k` is not a secret that must be protected in this context. The number of re-occurring patterns matched the number of loops the EEA took for the known values of `k` and `N`. From that match the researchers concluded that they in fact detected an EEA implementation.

## 4. Which Infineon products are confirmed to be vulnerable,
> and what is the potential broader impact?

All YubiKey 5 Series (with ﬁrmware version below 5.7) are impacted by the attack, moreover the Feitian A22 JavaCard (on which the researchers conducted their initial analysis). In addition to that, all Infineon SLB96xx TPMs are confirmed to be affected.

In addition to the confirmed vulnerable devices, the author states that all devices that use ECDSA and the Infineon cryptographic library are potentially affected. The following example systems are outlined in the paper, stating that the list is not comprehensive:
- FIDO and FIDO2 hardware tokens
- Crypto-Currency Hardware Wallets
- IC cards, Passports, Health Cards
- IOT devices which are built on the *Matter* standard (https://csa-iot.org/all-solutions/matter/)
- Vehicles, in their V2X interfaces they use to communicate with their environment (e.g. Vehicle to Grid, Vehicle to Vehicle, Vehicle to Cloud, ...)

## 5. Describe two mitigation measures recommended in the paper.

The author suggests these mitigation strategies

- Hardening the Infineon Cryptographic Library: This involves the improvement of the nonce blinding countermeasure by increasing the size of the multiplicative mask to the bit size of the elliptic curve. An alternative would be to replace EEA by a different inversion algorithm.
- High-Level Mitigations:
    - Not using ECDSA (if there is a choice)
    - Defense in Depth: Add an additional security layer, e.g. require a PIN or bio-metrics to make the security device services available
    - Protocol specific mitigations: E.g. a service can detect the usage of a counterfeit FIDO device by storing the latest authentication counter value provided by the security device. If a FIDO device successfully authenticates, but provides a lower or equal counter value than the recently stored one, this indicates that a counterfeit device was being used, and therefore the account shall be locked.
    
## 6. Propose a future research direction building on EUCLEAK’s findings.

For exploiting the weakness on YubiKeys, the researchers opened the casing to put the EM probe closer to the die. For a successful attack it is necessary that the YubiKey can be returned without the victim noticing that the casing was opened and closed again. Figure A.3 in the paper shows a re-packaged device where marks of the tampering are clearly visible.
An interesting research question to answer is how to solve this problem from an attackers point of view:
- Develop custom EM probes that can be placed further from the die, so the casing does not need to be opened
- Find a way to open the device without leaving tampering marks
- Counterfeit YubiKey casings for re-packaging such that the victim does not suspect their device was tampered with

## 7. Explain what is meant by a *non-constant-time* implementation
>of a cryptographic algorithm. Why does data-dependent execution time create a side-channel, and under what conditions can this leak secret information?

A non-constant time implementation of a cryptographic algorithm has the property that its execution time depends on the input data fed to the algorithm. A system leaks secret information to the outside of the system boundaries if that execution time can be measured from the outside. This can be done using EM probes, measuring the power consumption of the device is an alternative. It may even possible to derive the execution time from the latency measured between a request and obtaining the response (provided that there are no other random factors influencing the latency, thus masking the side channel).

If attackers can obtain timing measurements on input data that are known to or controlled by them, they can extract secret information out of the association (known Input -> measured execution time). The number of measured samples required for secret extraction depends on the exploited algorithm and its input and output data.

## 8. Discuss at least two generic strategies
>to convert a non-constant-time algorithm into a constant-time one (e.g., loop unrolling, Montgomery ladder, regularized arithmetic). What are the trade-offs in terms of performance and code complexity?

### Loop unrolling

Instead of implementing a loop with an exit condition that may leak information, the code block in the loop is copied `n` times in a row (`n` being the maximum number of loop iterations). As a consequence the `n` code blocks are executed sequentially without conditional branches depending on input data. As loop unrolling increases the code size, is only possible for small values of `n`.

Regarding performance, the resulting execution time will always consume the time of `n` iterations. However, it may even be a little faster than a loop iterating `n` times as no conditional branches for the loop are executed.

As loop unrolling removes the logic required for the unrolled loops, it slightly reduces code complexity. A linear loop `O(n)` is replaced by a constant `O(1)` block.

For loop unrolling, care must be taken that compilers won't optimize unrolled loops back to machine code that again leaks timing information. It is also important that the unrolled code blocks only contain code that is constant-time with respect to its input.

### Montgomery Ladder

The Montgomery Ladder is used for scalar multiplication in Elliptic Curve Cryptography. It is implemented as a loop, but the loop block itself yields a constant execution time so that no information leaks out of the loops execution time (code snippet taken from https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication):

```c
R0 ← 0
R1 ← P
for i from m downto 0 do
    if di = 0 then
        R1 ← point_add(R0, R1)
        R0 ← point_double(R0)
    else
        R0 ← point_add(R0, R1)
        R1 ← point_double(R1)

    // invariant property to maintain correctness
    assert R1 == point_add(R0, P)
return R0
```

Both branches in the loop execute an ECC point addition and an ECC point doubling independent of di, thereby not leaking the secret `d0..dm` bit sequence.

As for performance, the observed execution time will increase for all input data to the time required if the sequence `d0..dm` consists only one `1` bits. The Montgomery ladder has the same code complexity as the scalar multiplication algorithm that leaks information, `O(n)`.

### Masking

If a function `x=f(i)` leaks timing information about its secret input `i`, it may be possible to mask `i` by applying a function `g(i, r)` and its inverse, `g_inv(i, r)`, for instance like in the formula: `x=g_inv(f(g(i,r)),r)`. Applying `g()` has the effect that `f()` won't leak `i`, but a different value, `g(i,r)`. After the execution of `f()`, the original value `x` is obtained by applying the inverse function `g_inv()`. In order for this to work, the function `g()` must have an inverse `g_inv()`, and `r` must be a true random number (re-generated for each call of `f()`) which is large enough so it can't be brute forced by an attacker. It must also be verified that neither `g()` nor `g_inv()` leak information.

The influence on masking on performance and code complexity depends on the performance and code complexity of the introduced `g()` and `g_inv()` functions. Masking functions usually have a low complexity and performance penalty compared to the functions they are protecting.
