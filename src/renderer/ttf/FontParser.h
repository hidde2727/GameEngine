#ifndef ENGINE_RENDERER_TTF_FONTPARSER_H
#define ENGINE_RENDERER_TTF_FONTPARSER_H

// See https://learn.microsoft.com/en-us/typography/opentype/spec/ for all the information about .otf files
// and https://developer.apple.com/fonts/TrueType-Reference-Manual/ for all the information about .ttf files

#include "core/PCH.h"

#include "renderer/ttf/InstructionExecutor.h"

namespace Engine {
namespace Renderer {

	struct Characters {

		Characters() {}
		Characters(const char32_t c) {
			AddCharacter(c);
		}
		Characters(const char32_t start, const char32_t end) {
			AddCharacterRange(start, end);
		}
		Characters(const std::u32string str) {
			AddCharactersFromString(str);
		}

		// To input unicode characters use: AddCharacter(u'�')
		// Or use the unicode code: AddCharacters(u'\u00df')
		void AddCharacter(const char32_t c) {
			_characters.push_back(CharacterRange(c));
			_characterCount++;
		}

		// To input unicode characters use: AddCharacter(u'�')
		// Or use the unicode code: AddCharacters(u'\u00df')
		void AddCharacterRange(const char32_t start, const char32_t end) {
			_characters.push_back(CharacterRange(start, end));
			_characterCount += end - start + 1;
		}

		void AddCharactersFromString(const std::u32string in) {
			// Trying to condence the input by combine letters that are after each other (character code wise)
			char32_t start = 0; // Character 0 doesn't exist
			char32_t end = 0;
			for (char32_t charIn : in) {
				if (charIn == end + 1) {
					end++;
				}
				else {
					// Input into the storage
					if (start != 0) {
						_characters.push_back(CharacterRange(start, end));
					}
					// Start a new range
					start = charIn;
					end = charIn;
				}
			}
			_characters.push_back(CharacterRange(start, end));
			_characterCount += in.size();
		}

		bool Contains(const char32_t c) {
			for (CharacterRange range : _characters) {
				if (range.start >= c && range.end <= c) return true;
			}
			return false;
		}

		struct CharacterRange {
			CharacterRange() {}
			CharacterRange(const char32_t c) : start(c), end(c) {}
			CharacterRange(const char32_t start, const char32_t end) : start(start), end(end) {}
			char32_t start = 0;
			char32_t end = 0;
		};
		std::vector<CharacterRange> _characters;
		size_t _characterCount = 0;

		struct Iterator {
			Iterator(CharacterRange* range, bool end) {
				_currentRange = range;
				_character = end?_currentRange->end:_currentRange->start;
			}

			char32_t& operator*() { return _character; }
			char32_t* operator->() { return &_character; }

			Iterator& operator++() {
				if(_character + 1 > _currentRange->end) {
					_currentRange = _currentRange + 1;
					_character = _currentRange->start;
					return *this;
				}
				_character++;
				return *this;
			}
			Iterator operator++(int) { 
				Iterator ret(_currentRange, false);
				if(_character + 1 > _currentRange->end) {
					ret._currentRange = _currentRange + 1;
					ret._character = _currentRange->start;
					return ret;
				}
				ret._character = _character+1;
				return ret;
			}
		
			friend bool operator== (const Iterator& a, const Iterator& b) { return a._character == b._character; }
			friend bool operator!= (const Iterator& a, const Iterator& b) { 
				return a._character != b._character || a._currentRange != b._currentRange;
			}

			CharacterRange* _currentRange;
			char32_t _character;
		};
		Iterator begin() { return Iterator(&_characters[0], false); }
    	Iterator end()   { return Iterator((&_characters[_characters.size() - 1])+1, false); }

		char32_t operator[](const char32_t n) {
			char32_t i = 0;
			for(CharacterRange range : _characters) {
				if(i + (range.end - range.start) >= n) return range.start + (n-i);
				i += range.end - range.start + 1;
			}
			THROW("Cannot call operator[] on Characters with a n>size")
		}
	};

	class TTFFontParser {
	public:

		TTFFontParser();
		TTFFontParser(const std::string filePath, const Characters characters);

		// Will do the same as the constructor
		void LoadFile(const std::string filePath, const Characters characters);
		// Will set the characters to be extracted but will not parse them into memory
		void SetCharacters(const Characters characters);
		// Will reload the .ttf file but not parse it
		void LoadFile(const std::string filePath);
		// Will unload the raw .ttf file
		void UnloadFile();
		// Will reload the general tables that aren't dependent on the sfntVersion (type of encoding of the character outlines)
		void LoadGeneralTables();
		// Will reload the other tables than the general tables
		void LoadOtherTables();
		
		// For the getter function see below the private data

	private:

		void LoadTableOffsets();

		// General tables
		void LoadHEADTable();
		void LoadHHEATable();
		void LoadMAXPTable();
		void LoadHMTXTable();
		void LoadCMAPTable();
		void LoadKERNTable();

		// TTF tables
		void LoadLOCATable();
		void LoadGLYFTable();
		void LoadCVTable();
		void LoadFPGMTable();
		void LoadPREPTable();

		inline bool GoToTable(const std::array<uint8_t, 4> table){
			if (_tables.find(table) == _tables.end()) return false;
			_currentOffset = _tables[table]._offset;
			return true;
		}

		inline uint32_t GetTableSize(const std::array<uint8_t, 4> table) {
			if (_tables.find(table) == _tables.end()) return false;
			return _tables[table]._size;
		}

		inline bool GoToTableWithOffset(const std::array<uint8_t, 4> table, const uint16_t offset) {
			if(!GoToTable(table)) return false;
			_currentOffset += offset;
			return true;
		}

		inline void GoToOffset(const uint16_t offset) {
			_currentOffset = offset;
		}

		inline uint8_t	GetUint8()	{ return _data[_currentOffset++]; }
		inline int8_t	GetInt8()	{ return static_cast<int8_t>(GetUint8()); }
		inline uint16_t	GetUint16() { return ((uint16_t)GetUint8() << 8) | (uint16_t)GetUint8(); }
		inline int16_t	GetInt16()	{ return static_cast<int16_t>(GetUint16()); }
		inline uint32_t GetUint32() { return ((uint32_t)GetUint8() << 24) | ((uint32_t)GetUint8() << 16) | ((uint32_t)GetUint8() << 8) | (uint32_t)GetUint8(); }
		inline int32_t	GetInt32()	{ return static_cast<int32_t>(GetUint32()); }
		inline float	GetFixed()	{ return (float)GetInt32() / (float)(1 << 16); }
		inline float	GetF2DOT14(){ return (float)GetInt16() / (float)(1 << 14); }
		inline int16_t	GetFWord()	{ return GetInt16(); }
		inline uint16_t	GetUFWord() { return GetUint16(); }
		inline std::array<uint8_t, 4> GetTag() { return { GetUint8(), GetUint8(), GetUint8(), GetUint8() }; }
		inline uint16_t GetOffset16() { return GetUint16(); }
		inline uint32_t GetOffset32() { return GetUint32(); }
		inline void		GetDateTimePoint() { _currentOffset += 8; }
		inline uint32_t GetVersion16Dot16() { return GetUint32(); }

		std::vector<uint8_t> _data;
		size_t _currentOffset = 0;

		// ----------------------------------------------------------------------------------------
		// General data
		// ----------------------------------------------------------------------------------------

		// Characters to be extracted from the file
		Characters _characters;
		std::map<char32_t, uint16_t> _glyphIDs;
		std::map<uint16_t, char32_t> _reverseGlyphIDs;

		struct TTFTableOffset {

			TTFTableOffset() {}
			TTFTableOffset(uint32_t checkSum, uint32_t offset, uint32_t size) : _checkSum(checkSum), _offset(offset), _size(size) {}

			uint32_t _checkSum = 0;
			uint32_t _offset = 0;
			uint32_t _size = 0;
		};
		std::map<std::array<uint8_t, 4>, TTFTableOffset> _tables;

		// Data from the table directory
		uint32_t _sfntVersion;
		
		// HEAD data
		struct HEADTable {
			uint16_t _flags;
			uint16_t _unitsPerEm;
			int16_t  _xMin;
			int16_t  _yMin;
			int16_t  _xMax;
			int16_t  _yMax;
			uint16_t _macStyle;
			uint16_t _lowestRecPPEM;
			int16_t  _indexToLocFormat;
		} _head;

		// HHEA data
		struct HHEATable {
			int16_t  _ascender;
			int16_t  _descender;
			int16_t  _lineGap;
			uint16_t _advanceWidthMax;
			int16_t  _minLeftSideBearing;
			int16_t  _minRightSideBearing;
			int16_t  _xMaxExtent;
			int16_t  _caretSlopeRise;
			int16_t  _caretSlopeRun;
			int16_t  _caretOffset;
			uint16_t _numberOfHMetrics;
		} _hhea;

		// MAXP data 
		struct MAXPTable {
			bool _version1 = false;
			uint16_t _numGlyphs;
			uint16_t _maxPoints;
			uint16_t _maxContours;
			uint16_t _maxCompositePoints;
			uint16_t _maxCompositeContours;
			uint16_t _maxZones;
			uint16_t _maxTwilightPoints;
			uint16_t _maxStorage;
			uint16_t _maxFunctionDefs;
			uint16_t _maxInstructionDefs;
			uint16_t _maxStackElements;
			uint16_t _maxSizeOfInstructions;
			uint16_t _maxComponentElements;
			uint16_t _maxComponentDepth;
		} _maxp;

		// HTMX data
		struct HTMXTable {
			struct longHorMetric {
				uint16_t _advanceWidth;
				int16_t  _lsb;
			};
			std::vector<longHorMetric> _hMetrics;
			std::vector<int16_t> _leftSideBearings;
		} _hmtx;

		// KERN data
		struct KERNTable {

			// The kerning for text written in the horizontal direction, moves the text horizontally
			std::map<uint32_t, int16_t> _horizontal;
			// The kerning for text written in the horizontal direction, moves the text vertically
			std::map<uint32_t, int16_t> _vertical;

		} _kern;

		// ----------------------------------------------------------------------------------------
		// TTF data
		// ----------------------------------------------------------------------------------------

		// LOCA data
		struct LOCATable {
			std::vector<uint32_t> _locations;
		} _loca;

		public:
			struct BezierCurve {
				uint32_t degree;// 2, 3 or 4
				Util::Vec2F p1 = Util::Vec2F(0, 0);
				Util::Vec2F p2 = Util::Vec2F(0, 0);
				Util::Vec2F p3 = Util::Vec2F(0, 0);
				Util::Vec2F p4 = Util::Vec2F(0, 0);
			};
		private:

		// GLYF data
		struct GLYFTable {
			/*struct GlyphInfo {
				Util::Vec2I16 _min;
				Util::Vec2I16 _max;

				struct Contour {
					Contour(uint16_t contourStart, uint16_t contourLength) : _contourStart(contourStart), _contourLength(contourLength) {}

					uint16_t _contourStart;
					uint16_t _contourLength;
					Util::Vec2I16 _offset = Util::Vec2I16(0,0);
					Util::Vec2D _scale = Util::Vec2D(1,1);
				};
				std::vector<Contour> _contours;
				std::vector<uint8_t> _instructions;
			};
			std::map<char32_t, GlyphInfo> _glyphs;
			std::vector<BezierCurve> _curves;*/

			struct GlyphInfo {
				Util::Vec2F _min;
				Util::Vec2F _max;

				std::vector<uint16_t> _endPoints;
				std::vector<uint8_t> _flags;
				std::vector<uint8_t> _instructions;
				std::vector<Util::Vec2F> _points;
			};
			std::map<char32_t, GlyphInfo> _glyphs;
		} _glyf;

		// CVT data
		//struct CVTable {
		//	std::vector<int16_t> _values;
		//} _cvt;

		// FPGM data
		//struct FPGMTable {
		//	std::vector<uint8_t> _program;
		//} _fpgm;

		// PREP data
		struct PREPTable {
			std::vector<uint8_t> _program;
		} _prep;

		TTFInstructionExecutor _instructionExecutor;

		public: 

			bool IsTTF() { return _sfntVersion == 0x00010000 || _sfntVersion == 0x74727565; }
			bool IsCFF() { return _sfntVersion == 0x4F54544F; }
			bool IsOldPostScript() { return _sfntVersion == 0x74797031; }

			// Used to determine the factor all the points need to be multiplied with to receive their position in pixels
			// (All the data given is in FUnits (font design units not ready for direct rendering))
			double GetScale(uint32_t fontSizeInPixels) { return (1 / (double)_head._unitsPerEm) * (double)fontSizeInPixels; }
			uint16_t GetUnitsPerEm() { return _head._unitsPerEm; }

			uint16_t GetGlyphAdvance(const char32_t c);
			int16_t GetLeftSideBearing(const char32_t c);

			size_t GetNumberLoadedCharacters() { return _characters._characterCount; }

			int16_t GetHorizontalKerning(const char32_t left, const char32_t right);
			int16_t GetVerticalKerning(const char32_t left, const char32_t right);

			// TTF specific

			// The curves, if it is a lower degree bezier curve the p4 or even p3 will be set to (INT16_MIN, INT16_MIN)
			// This data is in FUnits so it needs to still be converted to pixels
			//std::vector<BezierCurve>* GetTTFCurves() { return &_glyf._curves; }

			// The data per glyph, is used to get the starting index in the glyph array and the size of the glyph
			//std::map<char32_t, GLYFTable::GlyphInfo>* GetTTFGlyphInfo() { return &_glyf._glyphs; }

			//size_t GetAmountTTFCurves() { return _glyf._curves.size(); }
			//size_t GetAmountTTFGlyphInfos() { return _glyf._glyphs.size(); }

			struct FontData {
				std::vector<BezierCurve> _curves;
				struct GlyphInfo {
					Util::Vec2F _min;
					Util::Vec2F _max;

					float _leftSideBearing;
					float _advance;
					std::map<char32_t, int16_t> _horizontalKerning;

					struct Contour {
						Contour(uint16_t contourStart, uint16_t contourLength) : _contourStart(contourStart), _contourLength(contourLength) {}

						uint16_t _contourStart;
						uint16_t _contourLength;
						Util::Vec2F _offset = Util::Vec2F(0, 0);
						Util::Vec2D _scale = Util::Vec2D(1, 1);
					};
					std::vector<Contour> _contours;
				};
				std::map<char32_t, GlyphInfo> _glyphs;
			};

			// Receive the data in FUnits, the font program won't be run
			std::unique_ptr<FontData> GetNonScaledData();
			// The font program will be run
			std::unique_ptr<FontData> GetScaledData(const uint32_t size);

		private:
			// Helper functions
			std::unique_ptr<TTFFontParser::FontData> GetGlyphData(std::map<char32_t, GLYFTable::GlyphInfo>* glyphInfo, const double renderScale);
	};

}
}

#endif