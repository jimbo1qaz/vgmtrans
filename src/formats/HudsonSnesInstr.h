#pragma once
#include "VGMInstrSet.h"
#include "VGMSampColl.h"
#include "VGMRgn.h"
#include "HudsonSnesFormat.h"

// ******************
// HudsonSnesInstrSet
// ******************

class HudsonSnesInstrSet :
	public VGMInstrSet
{
public:
	HudsonSnesInstrSet(RawFile* file, HudsonSnesVersion ver, uint32_t offset, uint32_t length, uint32_t spcDirAddr, const std::wstring & name = L"HudsonSnesInstrSet");
	virtual ~HudsonSnesInstrSet(void);

	virtual bool GetHeaderInfo();
	virtual bool GetInstrPointers();

	HudsonSnesVersion version;

protected:
	uint32_t spcDirAddr;
	std::vector<uint8_t> usedSRCNs;
};

// ***************
// HudsonSnesInstr
// ***************

class HudsonSnesInstr
	: public VGMInstr
{
public:
	HudsonSnesInstr(VGMInstrSet* instrSet, HudsonSnesVersion ver, uint32_t offset, uint8_t instrNum, uint32_t spcDirAddr, const std::wstring& name = L"HudsonSnesInstr");
	virtual ~HudsonSnesInstr(void);

	virtual bool LoadInstr();

	HudsonSnesVersion version;

protected:
	uint32_t spcDirAddr;
};

// *************
// HudsonSnesRgn
// *************

class HudsonSnesRgn
	: public VGMRgn
{
public:
	HudsonSnesRgn(HudsonSnesInstr* instr, HudsonSnesVersion ver, uint32_t offset, uint32_t spcDirAddr);
	virtual ~HudsonSnesRgn(void);

	virtual bool LoadRgn();

	HudsonSnesVersion version;
};
