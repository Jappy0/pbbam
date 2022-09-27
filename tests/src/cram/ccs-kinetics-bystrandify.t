-----------------------------------------
Check that zero-value clipping in CCS-Kinetics BAM works fine
-----------------------------------------

  $ "${CCS_KINETICS_BYSTRANDIFY}" \
  >   "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.bam \
  >   "${CRAMTMP}"/out.bam

  $ samtools view -h "${CRAMTMP}"/out.bam | grep -v '@PG	ID:samtools'
  @HD	VN:1.5	SO:unknown	pb:3.0.1
  @RG	ID:ab118ebd	PL:PACBIO	DS:READTYPE=CCS;Ipd:CodecV1=ip;PulseWidth:CodecV1=pw;BINDINGKIT=101-490-800;SEQUENCINGKIT=101-490-900;BASECALLERVERSION=5.0.0;FRAMERATEHZ=100.000000	PU:m64011_190228_190319	PM:SEQUELII	CM:S/P3-C1/5.0-8M
  @PG\tID:ccs-kinetics-bystrandify-*\tPN:ccs-kinetics-bystrandify\tVN:* (glob)
  m64011_190228_190319/1/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGTACG	aRWQRRRPWRA	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110,150,200,250	pw:B:C,2,6,11,21,31,41,101,111,151,201,251	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/1/ccs/rev	4	*	0	255	*	*	0	0	ACGTA	PRRRQ	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21	pw:B:C,110,100,40,30,20	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/fwd	4	*	0	255	*	*	0	0	TACGTACG	QRRRPWRA	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110,150,200,250	pw:B:C,21,31,41,101,111,151,201,251	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTA	ARWPRRRQ	cx:i:3	np:i:2	ip:B:C,251,201,151,111,101,41,31,21	pw:B:C,250,200,150,110,100,40,30,20	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGT	aRWQRRRP	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110	pw:B:C,2,6,11,21,31,41,101,111	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/rev	4	*	0	255	*	*	0	0	ACGTACGT	PRRRQWRa	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21,11,6,2	pw:B:C,110,100,40,30,20,10,5,1	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/fwd	4	*	0	255	*	*	0	0	TACGT	QRRRP	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110	pw:B:C,21,31,41,101,111	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTACGT	ARWPRRRQWRa	cx:i:3	np:i:4	ip:B:C,251,201,151,111,101,41,31,21,11,6,2	pw:B:C,250,200,150,110,100,40,30,20,10,5,1	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd

-----------------------------------------
Test coverage filter
-----------------------------------------

  $ "${CCS_KINETICS_BYSTRANDIFY}" \
  >   --min-coverage 3 \
  >   "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.bam \
  >   "${CRAMTMP}"/out-coverage.bam

  $ samtools view "${CRAMTMP}"/out-coverage.bam
  m64011_190228_190319/4/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTACGT	ARWPRRRQWRa	cx:i:3	np:i:4	ip:B:C,251,201,151,111,101,41,31,21,11,6,2	pw:B:C,250,200,150,110,100,40,30,20,10,5,1	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd

-----------------------------------------
ConsensusReadSet XML input & XML output
-----------------------------------------

  $ "${CCS_KINETICS_BYSTRANDIFY}" \
  >   "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml \
  >   "${CRAMTMP}"/xml-test-out.consensusreadset.xml

---- Input XML

  $ grep -c "ccs-kinetics-bystrandify-mock-input.bam\"" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml
  1
  $ grep -c "ccs-kinetics-bystrandify-mock-input.bam.pbi\"" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml
  1
  $ grep -c "<pbds:TotalLength>88</pbds:TotalLength>" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml
  1
  $ grep -c "<pbds:NumRecords>8</pbds:NumRecords>" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml
  1

---- Output XML

  $ grep -c "ccs-kinetics-bystrandify-mock-input.bystrand.bam\"" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1
  $ grep -c "ccs-kinetics-bystrandify-mock-input.bystrand.bam.pbi\"" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1
  $ grep -c "ccs-kinetics-bystrandify-mock-input.2.bystrand.bam\"" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1
  $ grep -c "ccs-kinetics-bystrandify-mock-input.2.bystrand.bam.pbi\"" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1
  $ grep -c "<pbds:TotalLength>128</pbds:TotalLength>" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1
  $ grep -c "<pbds:NumRecords>16</pbds:NumRecords>" "${CRAMTMP}"/xml-test-out.consensusreadset.xml
  1

---- Output BAM

  $ samtools view -h "${CRAMTMP}"/ccs-kinetics-bystrandify-mock-input.bystrand.bam | grep -v '@PG	ID:samtools'
  @HD	VN:1.5	SO:unknown	pb:3.0.1
  @RG	ID:ab118ebd	PL:PACBIO	DS:READTYPE=CCS;Ipd:CodecV1=ip;PulseWidth:CodecV1=pw;BINDINGKIT=101-490-800;SEQUENCINGKIT=101-490-900;BASECALLERVERSION=5.0.0;FRAMERATEHZ=100.000000	PU:m64011_190228_190319	PM:SEQUELII	CM:S/P3-C1/5.0-8M
  @PG\tID:ccs-kinetics-bystrandify-*\tPN:ccs-kinetics-bystrandify\tVN:* (glob)
  m64011_190228_190319/1/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGTACG	aRWQRRRPWRA	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110,150,200,250	pw:B:C,2,6,11,21,31,41,101,111,151,201,251	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/1/ccs/rev	4	*	0	255	*	*	0	0	ACGTA	PRRRQ	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21	pw:B:C,110,100,40,30,20	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/fwd	4	*	0	255	*	*	0	0	TACGTACG	QRRRPWRA	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110,150,200,250	pw:B:C,21,31,41,101,111,151,201,251	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTA	ARWPRRRQ	cx:i:3	np:i:2	ip:B:C,251,201,151,111,101,41,31,21	pw:B:C,250,200,150,110,100,40,30,20	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGT	aRWQRRRP	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110	pw:B:C,2,6,11,21,31,41,101,111	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/rev	4	*	0	255	*	*	0	0	ACGTACGT	PRRRQWRa	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21,11,6,2	pw:B:C,110,100,40,30,20,10,5,1	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/fwd	4	*	0	255	*	*	0	0	TACGT	QRRRP	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110	pw:B:C,21,31,41,101,111	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTACGT	ARWPRRRQWRa	cx:i:3	np:i:4	ip:B:C,251,201,151,111,101,41,31,21,11,6,2	pw:B:C,250,200,150,110,100,40,30,20,10,5,1	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd

  $ samtools view -h "${CRAMTMP}"/ccs-kinetics-bystrandify-mock-input.2.bystrand.bam | grep -v '@PG	ID:samtools'
  @HD	VN:1.5	SO:unknown	pb:3.0.1
  @RG	ID:ab118ebd	PL:PACBIO	DS:READTYPE=CCS;Ipd:CodecV1=ip;PulseWidth:CodecV1=pw;BINDINGKIT=101-490-800;SEQUENCINGKIT=101-490-900;BASECALLERVERSION=5.0.0;FRAMERATEHZ=100.000000	PU:m64011_190228_190319	PM:SEQUELII	CM:S/P3-C1/5.0-8M
  @PG\tID:ccs-kinetics-bystrandify-*\tPN:ccs-kinetics-bystrandify\tVN:* (glob)
  m64011_190228_190319/1/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGTACG	aRWQRRRPWRA	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110,150,200,250	pw:B:C,2,6,11,21,31,41,101,111,151,201,251	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/1/ccs/rev	4	*	0	255	*	*	0	0	ACGTA	PRRRQ	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21	pw:B:C,110,100,40,30,20	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/fwd	4	*	0	255	*	*	0	0	TACGTACG	QRRRPWRA	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110,150,200,250	pw:B:C,21,31,41,101,111,151,201,251	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTA	ARWPRRRQ	cx:i:3	np:i:2	ip:B:C,251,201,151,111,101,41,31,21	pw:B:C,250,200,150,110,100,40,30,20	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGT	aRWQRRRP	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110	pw:B:C,2,6,11,21,31,41,101,111	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/rev	4	*	0	255	*	*	0	0	ACGTACGT	PRRRQWRa	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21,11,6,2	pw:B:C,110,100,40,30,20,10,5,1	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/fwd	4	*	0	255	*	*	0	0	TACGT	QRRRP	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110	pw:B:C,21,31,41,101,111	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTACGT	ARWPRRRQWRa	cx:i:3	np:i:4	ip:B:C,251,201,151,111,101,41,31,21,11,6,2	pw:B:C,250,200,150,110,100,40,30,20,10,5,1	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd

-----------------------------------------
ConsensusReadSet XML input & BAM output
-----------------------------------------

---- Multi-BAM input fails for single BAM output

  $ "${CCS_KINETICS_BYSTRANDIFY}" \
  >   "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-mock-input.consensusreadset.xml \
  >   "${CRAMTMP}"/bam-out.bam
  * ERROR: Output is BAM. Input XML must only contain 1 input BAM file (glob)
  [1]

---- Single-BAM input OK
  $ "${CCS_KINETICS_BYSTRANDIFY}" \
  >   "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-single-input.consensusreadset.xml \
  >   "${CRAMTMP}"/bam-out.bam


---- Input XML

  $ grep -c "ccs-kinetics-bystrandify-mock-input.bam\"" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-single-input.consensusreadset.xml
  1
  $ grep -c "ccs-kinetics-bystrandify-mock-input.bam.pbi\"" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-single-input.consensusreadset.xml
  1
  $ grep -c "<pbds:TotalLength>44</pbds:TotalLength>" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-single-input.consensusreadset.xml
  1
  $ grep -c "<pbds:NumRecords>4</pbds:NumRecords>" "${TESTDIR}"/../../data/ccs-kinetics-bystrandify/ccs-kinetics-bystrandify-single-input.consensusreadset.xml
  1

---- Output BAM

  $ samtools view -h "${CRAMTMP}"/out.bam | grep -v '@PG	ID:samtools'
  @HD	VN:1.5	SO:unknown	pb:3.0.1
  @RG	ID:ab118ebd	PL:PACBIO	DS:READTYPE=CCS;Ipd:CodecV1=ip;PulseWidth:CodecV1=pw;BINDINGKIT=101-490-800;SEQUENCINGKIT=101-490-900;BASECALLERVERSION=5.0.0;FRAMERATEHZ=100.000000	PU:m64011_190228_190319	PM:SEQUELII	CM:S/P3-C1/5.0-8M
  @PG\tID:ccs-kinetics-bystrandify-*\tPN:ccs-kinetics-bystrandify\tVN:* (glob)
  m64011_190228_190319/1/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGTACG	aRWQRRRPWRA	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110,150,200,250	pw:B:C,2,6,11,21,31,41,101,111,151,201,251	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/1/ccs/rev	4	*	0	255	*	*	0	0	ACGTA	PRRRQ	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21	pw:B:C,110,100,40,30,20	zm:i:1	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/fwd	4	*	0	255	*	*	0	0	TACGTACG	QRRRPWRA	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110,150,200,250	pw:B:C,21,31,41,101,111,151,201,251	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/2/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTA	ARWPRRRQ	cx:i:3	np:i:2	ip:B:C,251,201,151,111,101,41,31,21	pw:B:C,250,200,150,110,100,40,30,20	zm:i:2	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/fwd	4	*	0	255	*	*	0	0	ACGTACGT	aRWQRRRP	cx:i:3	np:i:2	ip:B:C,1,5,10,20,30,40,100,110	pw:B:C,2,6,11,21,31,41,101,111	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/3/ccs/rev	4	*	0	255	*	*	0	0	ACGTACGT	PRRRQWRa	cx:i:3	np:i:2	ip:B:C,111,101,41,31,21,11,6,2	pw:B:C,110,100,40,30,20,10,5,1	zm:i:3	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/fwd	4	*	0	255	*	*	0	0	TACGT	QRRRP	cx:i:3	np:i:2	ip:B:C,20,30,40,100,110	pw:B:C,21,31,41,101,111	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd
  m64011_190228_190319/4/ccs/rev	4	*	0	255	*	*	0	0	CGTACGTACGT	ARWPRRRQWRa	cx:i:3	np:i:4	ip:B:C,251,201,151,111,101,41,31,21,11,6,2	pw:B:C,250,200,150,110,100,40,30,20,10,5,1	zm:i:4	sn:B:f,13.4413,20.9895,4.68673,8.39738	rq:f:0.999867	RG:Z:ab118ebd

