#include "renderer/ttf/FontParser.h"

namespace Engine {
namespace Renderer {

	TTFFontParser::TTFFontParser() {}

	TTFFontParser::TTFFontParser(const std::string filePath, const Characters characters) {
		LoadFile(filePath, characters);
	}
	void TTFFontParser::LoadFile(const std::string filePath, const Characters characters) {
		SetCharacters(characters);
		LoadFile(filePath);
		LoadGeneralTables();
		LoadOtherTables();
	}

	// Will set the characters to be extracted but will not parse them into memory
	void TTFFontParser::SetCharacters(const Characters characters) {
		_characters = characters;
	}
	// Will reload the .ttf file but not parse it
	void TTFFontParser::LoadFile(const std::string filePath) {
		std::ifstream input(filePath.c_str(), std::ifstream::binary);
		uint32_t dataSize = (uint32_t)std::filesystem::file_size(std::filesystem::path(filePath));
		_data.resize(dataSize);
		input.read((char*)_data.data(), dataSize);
	}
	// Will unload the raw .ttf file
	void TTFFontParser::UnloadFile() {
		_data.clear();
	}
	// Will reload the general tables that aren't dependent on the sfntVersion (type of encoding of the character outlines)
	void TTFFontParser::LoadGeneralTables() {
		// Loading the general tables that every font should have
		LoadTableOffsets();
		LoadHEADTable();
		LoadHHEATable();
		LoadMAXPTable();
		LoadHMTXTable();
		// Loading name table if we would want to
		// Loading OS/2 table if we would want to
		LoadCMAPTable();

		LoadKERNTable();
	}
	// Will reload the other tables than the general tables
	void TTFFontParser::LoadOtherTables() {
		if (IsTTF()) {
			// This is a TrueTypeFont

			_instructionExecutor.SetMaxStorageAreaSize(_maxp._maxStorage);
			_instructionExecutor.SetMaxFunctions(_maxp._maxFunctionDefs);
			_instructionExecutor.SetMaxTwilightPoints(_maxp._maxTwilightPoints);

			LoadLOCATable();
			LoadGLYFTable();
			LoadCVTable();
			LoadFPGMTable();
			LoadPREPTable();
			// GASP table
		}
		else if (IsCFF()) {
			// This is a CCF type font file
			THROW("[Renderer::TTFFontParser] Sorry this font file is currently not supported for parsing");
		}
		else if (IsOldPostScript()) {
			// This is a old style PostScript type font file
			THROW("[Renderer::TTFFontParser] Sorry this font file is currently not supported for parsing");
		}
		else {
			THROW("[Renderer::TTFFontParser] Illegal font file given, sfntVersion not known");
		}
	}

	void TTFFontParser::LoadTableOffsets() {
		_sfntVersion			= GetUint32();
		uint16_t numTables		= GetUint16();
		uint16_t searchRange	= GetUint16();
		uint16_t entrySelector	= GetUint16();
		uint16_t rangeShift		= GetUint16();

		for (uint16_t i = 0; i < numTables; i++) {
			std::array<uint8_t, 4> tag = GetTag();
			uint32_t checkSum = GetUint32();
			uint32_t offset = GetOffset32();
			uint32_t length = GetUint32();

			_tables[tag] = TTFTableOffset(checkSum, offset, length);
		}
	}

	void TTFFontParser::LoadHEADTable() {
		if (!GoToTable({ 'h', 'e', 'a', 'd' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'head' table");

		uint16_t majorVersion		= GetUint16();
		uint16_t minorVersion		= GetUint16();
		if (majorVersion != 1 || minorVersion != 0) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, head table version is not supported");
		}
		float    fontRevision		= GetFixed();
		uint32_t checksumAdjustment = GetUint32();
		uint32_t magicNumber		= GetUint32();
		_head._flags				= GetUint16();
		_head._unitsPerEm			= GetUint16();
		GetDateTimePoint();// Created
		GetDateTimePoint();// Modified
		_head._xMin					= GetInt16();
		_head._yMin					= GetInt16();
		_head._xMax					= GetInt16();
		_head._yMax					= GetInt16();
		_head._macStyle				= GetUint16();
		_head._lowestRecPPEM		= GetUint16();
		int16_t  fontDirectionHint	= GetInt16();
		_head._indexToLocFormat		= GetInt16();
		int16_t glyphDataFormat		= GetInt16();

		// Just make sure this font file isn't illegaly formatted
		if (magicNumber != 0x5F0F3CF5) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, magic number doesn't comply to standards");
		}
		else if (fontDirectionHint != 2 && fontDirectionHint != 1) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, font direction hint is deprecated and should always be 2");
		}
		else if (glyphDataFormat) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, unknown data format");
		}
	}

	void TTFFontParser::LoadHHEATable() {
		if (!GoToTable({ 'h', 'h', 'e', 'a' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'hhea' table");
		uint16_t majorVersion		= GetUint16();
		uint16_t minorVersion		= GetUint16();
		if (majorVersion != 1 || minorVersion != 0) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, hhea table version is not supported");
		}
		_hhea._ascender				= GetFWord();
		_hhea._descender			= GetFWord();
		_hhea._lineGap				= GetFWord();
		_hhea._advanceWidthMax		= GetUFWord();
		_hhea._minLeftSideBearing	= GetFWord();
		_hhea._minRightSideBearing	= GetFWord();
		_hhea._xMaxExtent			= GetFWord();
		_hhea._caretSlopeRise		= GetInt16();
		_hhea._caretSlopeRun		= GetInt16();
		_hhea._caretOffset			= GetInt16();
		if (GetInt16()) THROW("[Renderer::TTFFontParser] Illegal font file given, reserved slot is set, this parser is to old to handle that"); // This is a reserved slot
		if (GetInt16()) THROW("[Renderer::TTFFontParser] Illegal font file given, reserved slot is set, this parser is to old to handle that"); // This is a reserved slot
		if (GetInt16()) THROW("[Renderer::TTFFontParser] Illegal font file given, reserved slot is set, this parser is to old to handle that"); // This is a reserved slot
		if (GetInt16()) THROW("[Renderer::TTFFontParser] Illegal font file given, reserved slot is set, this parser is to old to handle that"); // This is a reserved slot
		if (GetInt16()) THROW("[Renderer::TTFFontParser] Illegal font file given, this parser only supports format 0") // This format should be 0
		_hhea._numberOfHMetrics = GetUint16();
	}

	void TTFFontParser::LoadMAXPTable() {
		if (!GoToTable({ 'm', 'a', 'x', 'p' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'maxp' table");

		_maxp._version1 = GetVersion16Dot16() == 0x00010000;
		
		_maxp._numGlyphs					= GetUint16();

		if (_maxp._version1) {
			_maxp._maxPoints				= GetUint16();
			_maxp._maxContours				= GetUint16();
			_maxp._maxCompositePoints		= GetUint16();
			_maxp._maxCompositeContours		= GetUint16();
			_maxp._maxZones					= GetUint16();
			_maxp._maxTwilightPoints		= GetUint16();
			_maxp._maxStorage				= GetUint16();
			_maxp._maxFunctionDefs			= GetUint16();
			_maxp._maxInstructionDefs		= GetUint16();
			_maxp._maxStackElements			= GetUint16();
			_maxp._maxSizeOfInstructions	= GetUint16();
			_maxp._maxComponentElements		= GetUint16();
			_maxp._maxComponentDepth		= GetUint16();
		}
	}

	void TTFFontParser::LoadHMTXTable() {
		if (!GoToTable({ 'h', 'm', 't', 'x' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'hmtx' table");

		_hmtx._hMetrics.resize(_hhea._numberOfHMetrics);
		_hmtx._leftSideBearings.resize(_maxp._numGlyphs - _hhea._numberOfHMetrics);

		for (size_t i = 0; i < _hmtx._hMetrics.size(); i++) {
			_hmtx._hMetrics[i]._advanceWidth = GetUint16();
			_hmtx._hMetrics[i]._lsb			 = GetInt16();
		}
		for (size_t i = 0; i < _hmtx._leftSideBearings.size(); i++) {
			_hmtx._leftSideBearings[i] = GetInt16();
		}
	}

#define KERN_HORIZONTAL 0x01
#define KERN_MINIMUM 0x02
#define KERN_CROSS_STREAM 0x04
#define KERN_OVERRIDE 0x08
#define KERN_FORMAT_MASK 0xF0
#define KERN_FORMAT_SHIFT 8

	void TTFFontParser::LoadKERNTable() {
		// This table is optional
		if (!GoToTable({ 'k', 'e', 'r', 'n' }))
			return;
		// This table shouldn't be present if the ttf file has CFF outlines
		if (IsCFF())
			return;
		uint16_t version = GetUint16();
		if (version != 0)
			THROW("[Renderer::TTFFontParser] Illegal font file given, kern table version is not supported");
		uint16_t numTables = GetUint16();
		for (int i = 0; i < numTables; i++) {
			uint64_t tableStartOffset = _currentOffset;

			uint16_t subVersion = GetUint16();
			uint16_t length = GetUint16();
			uint16_t coverage = GetUint16();

			// Skip this table if it contains vertical kerning or minimums
			if (!(coverage & KERN_HORIZONTAL) || coverage & KERN_MINIMUM) {
				_currentOffset = tableStartOffset + length;
				continue;
			}
			uint8_t format = (coverage & KERN_FORMAT_MASK) << KERN_FORMAT_SHIFT;
			if (format == 0) {
				uint16_t nPairs = GetUint16();
				uint16_t searchRange = GetUint16();
				uint16_t entrySelector = GetUint16();
				uint16_t rangeShift = GetUint16();

				for (int j = 0; j < nPairs; j++) {
					uint16_t left = GetUint16();
					uint16_t right = GetUint16();
					int16_t value = GetFWord();
					
					// Check if we need to store these values
					char32_t leftChar = _reverseGlyphIDs[left];
					char32_t rightChar = _reverseGlyphIDs[right];

					bool foundLeft = false;
					bool foundRight = false;
					for (Characters::CharacterRange range : _characters._characters) {
						if (range.start <= leftChar && range.end >= leftChar)
							foundLeft = true;
						else if (range.start <= rightChar && range.end >= rightChar)
							foundRight = true;

						if (foundLeft && foundRight)
							break;
					}
					if (!(foundLeft && foundRight))
						continue;

					// We already know the coverage is horizontal, only need to check if it is cross or non-cross stream
					if (coverage & KERN_CROSS_STREAM)
						_kern._vertical[((uint32_t)left << 16) | (uint32_t)right] = value;
					else
						_kern._horizontal[((uint32_t)left << 16) | (uint32_t)right] = value;
				}
			}
			else if (format == 2) {
				THROW("[Renderer::TTFFontParser] Font file, the KERN subtable format hasn't been implemented yet");
				uint16_t rowWidth = GetUint16();
				uint16_t leftClassOffset = GetOffset16();
				uint16_t rightClassOffset = GetOffset16();
				uint16_t kerningArrayOffset = GetOffset16();
			}
			else {
				THROW("[Renderer::TTFFontParser] Illegal font file given, kern subtable format is not supported")
			}
		}
	}

	void TTFFontParser::LoadCMAPTable() {
		if (!GoToTable({ 'c', 'm', 'a', 'p' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'cmap' table");

		uint16_t version	= GetUint16();
		if (version) {
			THROW("[Renderer::TTFFontParser] Illegal font file given, cmap table version is not supported");
		}
		uint16_t numTables	= GetUint16();
		struct EncodingRecord {
			uint16_t platformID;
			uint16_t encodingID;
			uint32_t subTableOffset;
		};
		std::vector<EncodingRecord> records;
		records.resize(numTables);
		for (size_t i = 0; i < records.size(); i++) {
			records[i].platformID		= GetUint16();
			records[i].encodingID		= GetUint16();
			records[i].subTableOffset	= GetOffset32();
		}

		for (EncodingRecord record : records) {
			GoToTableWithOffset({ 'c', 'm', 'a', 'p' }, record.subTableOffset); // We already know the table exists so can ignore the result
			uint16_t format = GetUint16();
			// Yes, there are a lot of formats
			if (format == 0) {
				// This format is used for 'older Macintosh platforms'
				continue;
			}
			else if (format == 2) {
				// The documentation states that 'This format is not commonly used today'
				continue;
			}
			else if (format == 4) {
				uint16_t length			= GetUint16();
				uint16_t language		= GetUint16();
				uint16_t segCountX2		= GetUint16();
				uint16_t segCount		= segCountX2 / 2;
				uint16_t searchRange	= GetUint16();
				uint16_t entrySelector	= GetUint16();
				uint16_t rangeShift		= GetUint16();
				std::vector<uint16_t> endCodes;
				endCodes.resize(segCount);
				for (size_t i = 0; i < segCount; i++) {
					endCodes[i] = GetUint16();
				}

				if (GetUint16()) THROW("[Renderer::TTFFontParser] Illegal font file given, reserved slot is set, this parser is to old to handle that"); // This is a reserved slot

				std::vector<uint16_t> startCodes;
				startCodes.resize(segCount);
				for (size_t i = 0; i < segCount; i++) {
					startCodes[i] = GetUint16();
				}

				std::vector<uint16_t> idDeltas;
				idDeltas.resize(segCount);
				for (size_t i = 0; i < segCount; i++) {
					idDeltas[i] = GetUint16();
				}

				uint16_t idRangeStart = (uint16_t)_currentOffset;

				std::vector<uint16_t> idRangeOffsets;
				idRangeOffsets.resize(segCount);
				for (size_t i = 0; i < segCount; i++) {
					idRangeOffsets[i] = GetUint16();
				}

				// Input the information into the _glyphIDs map
				for (const char32_t c : _characters) {
					// Start search at the front of the array, no optimization at the moment
					for (uint16_t i = 0; i < segCount; i++) {
						if (endCodes[i] < c) continue;

						if (startCodes[i] > c) break; // The character doesn't exist in this font
						uint16_t glyphID = 0;
						if (idRangeOffsets[i] != 0) {
							GoToOffset(idRangeStart + i * 2
								+ idRangeOffsets[i]
								+ (((uint16_t)c) - startCodes[i]) * 2);
							glyphID = GetUint16();
							if (glyphID == 0) break; // Missing glyph indicator
							glyphID += idDeltas[i];
						}
						else {
							glyphID = c + idDeltas[i];
						}
						_glyphIDs[c] = glyphID;
						_reverseGlyphIDs[glyphID] = c;
						break;
					}
				}
			}
			else if (format == 6) {
				WARNING("[Renderer::TTFFontParser] The CMAP format type hasn't been tested yet, please proceed with caution");
				uint16_t length		= GetUint16();
				uint16_t language	= GetUint16();
				uint16_t firstCode	= GetUint16();
				uint16_t entryCount = GetUint16();
				for (uint16_t i = 0; i < entryCount; i++) {
					uint16_t glyphID = GetUint16();
					if(_characters.Contains(firstCode + i)) {
						_glyphIDs[firstCode+i] = glyphID;
					}
				}
			}
			else if (format == 8) {
				WARNING("[Renderer::TTFFontParser] Font file contains a format not yet implemented");
			}
			else if (format == 10) {
				WARNING("[Renderer::TTFFontParser] Font file contains a format not yet implemented");
			}
			else if (format == 12) {
				WARNING("[Renderer::TTFFontParser] Font file contains a format not yet implemented");
			}
			else if (format == 13) {
				WARNING("[Renderer::TTFFontParser] Font file contains a format not yet implemented");
			}
			else if (format == 14) {
				WARNING("[Renderer::TTFFontParser] Font file contains a format not yet implemented");
			}
		}
	}

	void TTFFontParser::LoadLOCATable() {
		if (!GoToTable({ 'l', 'o', 'c', 'a' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'loca' table but is specified as a TTF file");
		uint16_t amountGlyphs = _maxp._numGlyphs;
		if (_head._indexToLocFormat == 0) {
			for (uint16_t i = 0; i < amountGlyphs; i++) {
				_loca._locations.push_back(GetOffset16());
			}
		}
		else if (_head._indexToLocFormat == 1) {
			for (uint16_t i = 0; i < amountGlyphs; i++) {
				_loca._locations.push_back(GetOffset32());
			}
		} 
		else {
			THROW("[Renderer::TTFFontParser] Illegal font file given, loca format not supported");
		}
	}

	#define GLYF_ON_CURVE_POINT 0x01
	#define GLYF_X_SHORT_VECTOR 0x02
	#define GLYF_Y_SHORT_VECTOR 0x04
	#define GLYF_REPEAT_FLAG 0x08
	#define GLYF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
	#define GLYF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
	#define GLYF_OVERLAP_SIMPLE 0x40
	#define GLYF_RESERVED 0x80

	void TTFFontParser::LoadGLYFTable() {
		if (!GoToTable({ 'g', 'l', 'y', 'f' }))
			THROW("[Renderer::TTFFontParser] Illegal font file given, file doesn't contain the 'glyf' table but is specified as a TTF file");
		// Go through all the requested characters
		_glyf._glyphs.clear();
		for (const char32_t c : _characters) {
			GLYFTable::GlyphInfo* info = &_glyf._glyphs[c];

			if (_loca._locations[_glyphIDs[c]] == _loca._locations[_glyphIDs[c] + 1]) {
				info->_min = Util::Vec2I16(0, 0);
				info->_max = Util::Vec2I16(0,0);
				continue;
			}

			GoToTableWithOffset({ 'g', 'l', 'y', 'f' },
			_loca._locations[_glyphIDs[c]] * (_head._indexToLocFormat ? 1 : 2));
			int16_t numberOfContours	= GetInt16();
			info->_min.x		= GetInt16();
			info->_min.y		= GetInt16();
			info->_max.x		= GetInt16();
			info->_max.y		= GetInt16();
			if (numberOfContours > 0) {
				// Endpoints
				for (uint16_t i = 0; i < numberOfContours; i++) {
					info->_endPoints.push_back(GetUint16());
				}
				uint16_t amountPoints = info->_endPoints.at(info->_endPoints.size() - 1);

				// Instructions
				uint16_t instructionLength = GetUint16();
				_glyf._glyphs[c]._instructions.reserve(instructionLength);
				for (size_t i = 0; i < instructionLength; i++) {
					_glyf._glyphs[c]._instructions.push_back(GetUint8());
				}

				// Flags
				info->_flags.reserve(amountPoints);
				size_t coordinateXSize = 0;
				while (info->_flags.size() < amountPoints + 1) {
					uint8_t flag = GetUint8();
					uint8_t repeats = 1;
					if (flag & GLYF_REPEAT_FLAG)
						repeats = GetUint8() + 1;
					info->_flags.insert(info->_flags.end(), repeats, flag);

					if (flag & GLYF_X_SHORT_VECTOR)
						coordinateXSize += repeats;
					else if (flag & GLYF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
						coordinateXSize;
					else
						coordinateXSize += repeats * 2;
					// We don't need to know the size of the Y array
				}

				// Points
				size_t xPosStart = _currentOffset;
				size_t xIndex = 0;
				size_t yIndex = 0;
				Util::Vec2I16 pos;
				for(uint8_t flag : info->_flags) {
					// X coordinate
					if (flag & GLYF_X_SHORT_VECTOR) {
						// It is 8 bit
						_currentOffset = xPosStart + xIndex;
						pos.x += GetUint8() * (flag & GLYF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR ? 1 : -1);
						xIndex++;
					}
					else if (flag & GLYF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) {
						// It is the same as the previous
						// Do nothing
					}
					else {
						// It is 16 bit
						_currentOffset = xPosStart + xIndex;
						pos.x += GetInt16();
						xIndex += 2;
					}

					// Y coordinate
					if (flag & GLYF_Y_SHORT_VECTOR) {
						// It is 8 bit
						_currentOffset = xPosStart + coordinateXSize + yIndex;
						pos.y += GetUint8() * (flag & GLYF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR ? 1 : -1);
						yIndex++;
					}
					else if (flag & GLYF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) {
						// It is the same as the previous
						// Do nothing
					}
					else {
						// It is 16 bit
						_currentOffset = xPosStart + coordinateXSize +yIndex;
						pos.y += GetInt16();
						yIndex += 2;
					}
					info->_points.push_back(pos);
				}
			}
			else if (numberOfContours == -1) {
				WARNING("[Renderer::TTFFontParser] Sorry, font files with composite glyphs aren't supported at the moment");
			}
			else {
				THROW("[Renderer::TTFFontParser] Illegal font file given, numberOfContours contains an illegal value");
			}
		}
	}

	void TTFFontParser::LoadCVTable() {
		// This table is optional
		if (!GoToTable({ 'c', 'v', 't', ' ' }))
			return;
		// This table shouldn't be present if the ttf file doesn't contain TTF outlines
		if (!IsTTF())
			return;
		uint32_t tableSize = GetTableSize({ 'c', 'v', 't', ' ' });
		// Values are 2 bytes long, so devide the size by 2
		tableSize /= 2;
		_instructionExecutor.ReserveControlValueSpace(tableSize);
		for (uint32_t i = 0; i < tableSize; i++) {
			_instructionExecutor.AddControlValue(GetFWord());
		}
	}

	void TTFFontParser::LoadFPGMTable() {
		// This table is optional
		if (!GoToTable({ 'f', 'p', 'g', 'm' }))
			return;
		// This table shouldn't be present if the ttf file doesn't contain TTF outlines
		if (!IsTTF())
			return;
		uint32_t tableSize = GetTableSize({ 'f', 'p', 'g', 'm' });
		_instructionExecutor.ReserveInstructionStreamSpace(tableSize);
		for (uint32_t i = 0; i < tableSize; i++) {
			_instructionExecutor.AddToInstructionStream(GetUint8());
		}
		try {
			_instructionExecutor.ExecuteStack();
		} catch(std::exception exc) {
			WARNING("[Renderer::TTFFontParser] Failed to execute instructions of the font-program, executor returned the error:\n" + std::string(exc.what()))
		}
	}

	void TTFFontParser::LoadPREPTable() {
		// This table is optional
		if (!GoToTable({ 'p', 'r', 'e', 'p' }))
			return;
		// This table shouldn't be present if the ttf file doesn't contain TTF outlines
		if (!IsTTF())
			return;
		uint32_t tableSize = GetTableSize({ 'p', 'r', 'e', 'p' });
		_prep._program.reserve(tableSize);
		for (uint32_t i = 0; i < tableSize; i++) {
			_prep._program.push_back(GetUint8());
		}
	}


	uint16_t TTFFontParser::GetGlyphAdvance(const char32_t c) {
		uint16_t id = _glyphIDs[c];
		if (id >= _hmtx._hMetrics.size()) {
			// Only has a left side bearing
			THROW("[Renderer::TTFFontParser] Glyph doesn't have an advance");
		}
		else {
			// Has a left side bearing and an advance
			return _hmtx._hMetrics[id]._advanceWidth;
		}
	}

	int16_t TTFFontParser::GetLeftSideBearing(const char32_t c) {
		uint16_t id = _glyphIDs[c];
		if (id >= _hmtx._hMetrics.size()) {
			// Only has a left side bearing
			return _hmtx._leftSideBearings[id - _hmtx._hMetrics.size()];
		}
		else {
			// Has a left side bearing and an advance
			return _hmtx._hMetrics[id]._lsb;
		}
	}

	int16_t TTFFontParser::GetHorizontalKerning(const char32_t left, const char32_t right) {
		uint16_t leftID = _glyphIDs[left];
		uint16_t rightID = _glyphIDs[right];

		if(_kern._horizontal.contains(((uint32_t)leftID << 16) | rightID))
			return _kern._horizontal[((uint32_t)leftID << 16) | rightID];
		else
			return 0;
	}

	int16_t TTFFontParser::GetVerticalKerning(const char32_t left, const char32_t right) {
		uint16_t leftID = _glyphIDs[left];
		uint16_t rightID = _glyphIDs[right];

		if (_kern._vertical.contains(((uint32_t)leftID << 16) | rightID))
			return _kern._vertical[((uint32_t)leftID << 16) | rightID];
		else
			return 0;
	}

	std::unique_ptr<TTFFontParser::FontData> TTFFontParser::GetNonScaledData() {
		return GetGlyphData(&_glyf._glyphs, 1.f);
	}

	std::unique_ptr<TTFFontParser::FontData> TTFFontParser::GetScaledData(const uint32_t size) {
		TTFInstructionExecutor executor = _instructionExecutor; // Yes, copying the executor, we may modify the executor and we don't want to have to reexecute the fpgm
		double scale = GetScale(size);
		executor.SetScale(_head._unitsPerEm, size);
		executor.AddToInstructionStream(&_prep._program);
		try {
			executor.ExecuteStack();
		} catch(std::exception exc) {
			WARNING("[Renderer::TTFFontParser] Failed to execute instructions of the PREP, executor returned the error:\n" + std::string(exc.what()))
		}
		executor.StoreGraphicsState();

		std::map<char32_t, GLYFTable::GlyphInfo> glyphs;
		for (auto const& [c, info] : _glyf._glyphs) {

			executor.BindStoredGraphicsState();
			executor.SetOriginalGlyphInfo(info._flags, info._points, info._endPoints);
			executor.AddPhantomPoints(info._min, info._max, GetLeftSideBearing(c), GetGlyphAdvance(c), 0, 0);
			executor.AddToInstructionStream(&info._instructions);

			try {
				executor.ExecuteStack();
			} catch(std::exception exc) {
				WARNING("[Renderer::TTFFontParser] Failed to execute instructions of '" + std::string(1, c) + "', executor returned the error:\n" + std::string(exc.what()))
			}

			glyphs[c]._min = Util::Vec2F(info._min.x, info._min.y);
			glyphs[c]._max = Util::Vec2F(info._max.x, info._max.y);
			glyphs[c]._endPoints = info._endPoints;
			glyphs[c]._flags = *executor.GetNewFlags();
			glyphs[c]._points = *executor.GetNewPoints();
		}

		return GetGlyphData(&glyphs, scale);
	}

	std::unique_ptr<TTFFontParser::FontData> TTFFontParser::GetGlyphData(std::map<char32_t, GLYFTable::GlyphInfo>* glyphInfo, const double renderScale) {
		std::unique_ptr<FontData> data = std::make_unique<FontData>();

		uint32_t curveIndex = 0;

		for (auto const& [c, info] : *glyphInfo) {
			FontData::GlyphInfo* glyphInfo = &data->_glyphs[c];

			glyphInfo->_min = info._min*renderScale;
			glyphInfo->_max = info._max*renderScale;
			glyphInfo->_advance = GetGlyphAdvance(c)*renderScale;
			glyphInfo->_leftSideBearing = GetLeftSideBearing(c)*renderScale;
			// TODO: horizontal/vertical kerning

			if (info._min == Util::Vec2F(0,0) && info._max == Util::Vec2F(0, 0)) continue;

			data->_curves.push_back(BezierCurve());

			uint32_t index = 0;
			uint32_t contourIndex = 0;
			uint32_t currentContourStart = curveIndex;
			uint32_t pointIndex = 1;

			for (Util::Vec2F p : info._points) {
				// Check if this is the start of a new contour
				if (info._endPoints[contourIndex] + 1 == index) {
					// Start a new contour
					switch (pointIndex) {
					case 2: data->_curves[curveIndex].p2 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 2; break;
					case 3: data->_curves[curveIndex].p3 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 3; break;
					case 4: data->_curves[curveIndex].p4 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 4; break;
					}
					contourIndex++;
					curveIndex++;
					data->_curves.push_back(BezierCurve());
					data->_curves[curveIndex].p1 = p;
					pointIndex = 1;
					// Insert the previous contour
					glyphInfo->_contours.push_back(FontData::GlyphInfo::Contour(currentContourStart, curveIndex - currentContourStart));
					currentContourStart = curveIndex;
				}
				// Insert the coordinates into the arrays
				else if ((info._flags[index] & GLYF_ON_CURVE_POINT && pointIndex != 1) || pointIndex == 4) {
					// New curve start
					switch (pointIndex) {
					case 2: data->_curves[curveIndex].p2 = p; data->_curves[curveIndex].degree = 2; break;
					case 3: data->_curves[curveIndex].p3 = p; data->_curves[curveIndex].degree = 3; break;
					case 4: data->_curves[curveIndex].p4 = p; data->_curves[curveIndex].degree = 4; break;
					}
					curveIndex++;
					data->_curves.push_back(BezierCurve());
					data->_curves[curveIndex].p1 = p;
					pointIndex = 1;
				}
				else {
					switch (pointIndex) {
					case 1: data->_curves[curveIndex].p1 = p; break;
					case 2: data->_curves[curveIndex].p2 = p; break;
					case 3: data->_curves[curveIndex].p3 = p; break;
					}
				}
				pointIndex++;
				index++;
			}
			// Finish the last contour
			switch (pointIndex) {
			case 2: data->_curves[curveIndex].p2 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 2; break;
			case 3: data->_curves[curveIndex].p3 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 3; break;
			case 4: data->_curves[curveIndex].p4 = data->_curves[currentContourStart].p1; data->_curves[curveIndex].degree = 4; break;
			}
			curveIndex++;
			glyphInfo->_contours.push_back(FontData::GlyphInfo::Contour(currentContourStart, curveIndex - currentContourStart));
		}
		data->_curves.push_back(BezierCurve());

		return data;
	}

}
}