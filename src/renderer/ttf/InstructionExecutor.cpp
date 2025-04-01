#include "renderer/ttf/InstructionExecutor.h"

namespace Engine {
namespace Renderer {

	// ---------------------------------------------------
	// Public
	// ---------------------------------------------------
	void TTFInstructionExecutor::SetMaxStorageAreaSize(const size_t size) {
		_storageArea.resize(size);
	}
	void TTFInstructionExecutor::SetMaxFunctions(const size_t size) {
		_functions.resize(size);
	}
	void TTFInstructionExecutor::SetMaxTwilightPoints(const size_t size) {
		_state._twilightPoints.resize(size);
		_state._originalTwilightPoints.resize(size);
	}
	void TTFInstructionExecutor::SetScale(const uint32_t unitsPerEM, const uint32_t pointSize) {
		_state._pointScale = (1 / (double)unitsPerEM) * (double)pointSize;
		_state._pointSize = pointSize;
		_state._pixelsPerEM = (uint32_t)(unitsPerEM * _state._pointScale);
		if (_controlValuesScaled)
			THROW("Please use a version of the TTFInstructionExecutor where the CVT table hasn't been scaled");
		for (F26Dot6& value : _controlValues) {
			value = (F26Dot6)(value * _state._pointScale);
		}
		_controlValuesScaled = true;
	}
	void TTFInstructionExecutor::SetOriginalGlyphInfo(const std::vector<uint8_t> flags, const std::vector<Utils::Vec2F> points, const std::vector<uint16_t> endpoints) {
		_flags = flags;
		_originalFlags.reserve(_flags.size());
		for (uint8_t flag : _flags) {
			_originalFlags.push_back(flag & GLYF_ON_CURVE_POINT); // Only keep the on curve flag, the rest of the space for flags gets used for internal things
		}

		_points.reserve(points.size() + 4);
		_points.resize(points.size());
		size_t i = 0;
		for (Utils::Vec2I16 point : points) {
			_points[i].x = ToF26Dot6(point.x * _state._pointScale);
			_points[i].y = ToF26Dot6(point.y * _state._pointScale);

			i++;
		}
		_originalPoints = _points;
		_state._IUPDeltas.resize(points.size());

		_endpoints = endpoints;
	}
	void TTFInstructionExecutor::AddPhantomPoints(const Utils::Vec2F min, const Utils::Vec2F max, const int32_t leftSideBearing, const int32_t advance, const int32_t topOrigin, const int32_t advanceHeight) {
		// !UNDOCUMENTED! taken from the FreeType project
		// Fantom points aren't what they say in the documentation
		// TODO, if subpixel then also set the x of point 3 and 4 =advanceScaled/2
		// ----------------------------------------------
		Utils::Vec2<F26Dot6> minScaled = ToF26Dot6(min * _state._pointScale);
		Utils::Vec2<F26Dot6> maxScaled = ToF26Dot6(max * _state._pointScale);
		F26Dot6 leftSideBearingScaled = ToF26Dot6(leftSideBearing * _state._pointScale);
		F26Dot6 advanceScaled = ToF26Dot6(advance * _state._pointScale);
		F26Dot6 topOriginScaled = ToF26Dot6(topOrigin * _state._pointScale);
		F26Dot6 advanceHeightScaled = ToF26Dot6(advanceHeight * _state._pointScale);

		_originalPoints.push_back({ minScaled.x - leftSideBearingScaled, 0 });
		_points.push_back({ minScaled.x - leftSideBearingScaled, 0 });
		_originalPoints.push_back({ _points[_points.size() - 1].x + advanceScaled, 0 });
		_points.push_back({ _points[_points.size() - 1].x + advanceScaled, 0 });
		_originalPoints.push_back({ 0, maxScaled.y + topOriginScaled });
		_points.push_back({ 0, maxScaled.y + topOriginScaled });
		_originalPoints.push_back({ 0, _points[_points.size() - 1].y - advanceHeightScaled });
		_points.push_back({ 0, _points[_points.size() - 1].y - advanceHeightScaled });

		// !UNDOCUMENTED! taken from the FreeType project
		// Fantom points are rounded to the grid
		// ----------------------------------------------
		for (size_t i = _points.size() - 4; i < _points.size(); i++) {
			_points[i] = RoundToGrid(_points[i]);
			_originalPoints[i] = RoundToGrid(_originalPoints[i]);
		}
	}

	// Getters
	std::unique_ptr<std::vector<Utils::Vec2F>> TTFInstructionExecutor::GetNewPoints() {
		bool hasIUPDeltas = !_state._IUPDeltas.empty();

		std::unique_ptr<std::vector<Utils::Vec2F>> points = std::make_unique<std::vector<Utils::Vec2F>>();
		points->reserve(_points.size());
		for (size_t i = 0; i < _points.size() - 4; i++) {
			if (hasIUPDeltas && (_flags[i] & GLYF_TOUCHED_X) && (_flags[i] & GLYF_TOUCHED_Y))
				points->push_back(FromF26Dot6<float>(_points[i]));
			else if (hasIUPDeltas && (_flags[i] & GLYF_TOUCHED_X))
				points->push_back(FromF26Dot6<float>(_points[i] + Utils::Vec2<F26Dot6>(0, _state._IUPDeltas[i].y)));
			else if (hasIUPDeltas && (_flags[i] & GLYF_TOUCHED_Y))
				points->push_back(FromF26Dot6<float>(_points[i] + Utils::Vec2<F26Dot6>(_state._IUPDeltas[i].x, 0)));
			else if (hasIUPDeltas)
				points->push_back(FromF26Dot6<float>(_points[i] + _state._IUPDeltas[i]));
			else
				points->push_back(FromF26Dot6<float>(_points[i]));
		}
		return points;
	}
	// Used to store after the FPGM and CVT program
	void TTFInstructionExecutor::StoreGraphicsState() {
		// !UNDOCUMENTED! taken from the FreeType project
		// These variables shouldn't be stored after the FPGM and CVT program

		_state._dualProjectionVector = Utils::Vec2F(1, 0);
		_state._projectionVector = Utils::Vec2F(1, 0);
		_state._freedomVector = Utils::Vec2F(1, 0);

		_state._refrencePoint0 = 0;
		_state._refrencePoint1 = 0;
		_state._refrencePoint2 = 0;

		_state._zonePointer0 = 1;
		_state._zonePointer1 = 1;
		_state._zonePointer2 = 1;

		_state._loopVariable = 1;
		// ----------------------------------------------

		_storedState = _state;
	}
	void TTFInstructionExecutor::BindStoredGraphicsState() {
		_state = _storedState;
	}

	// ---------------------------------------------------
	// Helpers
	// ---------------------------------------------------

	inline bool TTFInstructionExecutor::IsNextInstructionLeft() {
		if (_currentlyExecutingFunctions.empty())
			return _currentInstruction < _instructionStream.size();
		else
			return _currentlyExecutingFunctions.front()._currentInstruction < _functions[_currentlyExecutingFunctions.front()._function].size();
	}
	inline uint8_t TTFInstructionExecutor::GetNextInstructionByte() {
		if (_currentlyExecutingFunctions.empty())
			return _instructionStream[_currentInstruction++];
		else
			return _functions[_currentlyExecutingFunctions.front()._function][_currentlyExecutingFunctions.front()._currentInstruction++];
	}
	inline int32_t TTFInstructionExecutor::GetNextInstructionPaddedWord() {
		uint8_t highByte = GetNextInstructionByte();
		uint8_t lowByte = GetNextInstructionByte();
		return (int32_t)static_cast<int16_t>((highByte << 8) | lowByte);
	}

	inline void TTFInstructionExecutor::JumpForwardAmountInstruction(const int32_t amount) {
		if (_currentlyExecutingFunctions.empty())
			_currentInstruction += amount;
		else
			_currentlyExecutingFunctions.front()._currentInstruction += amount;
	}

	inline void TTFInstructionExecutor::PushStack(const bool value) {
		PushStack((uint32_t)value);
	}
	inline void TTFInstructionExecutor::PushStack(const uint8_t value) {
		_interpreterStack.push_front(value);
	}
	inline void TTFInstructionExecutor::PushStack(const int32_t value) {
		_interpreterStack.push_front(value);
	}
	inline void TTFInstructionExecutor::PushStack(const uint32_t value) {
		_interpreterStack.push_front(static_cast<int32_t>(value));
	}
	inline int32_t TTFInstructionExecutor::GetNextStackInt32() {
		if (_interpreterStack.empty())
			THROW("Can't retrieve a value of an empty stack");
		int32_t value = _interpreterStack.front();
		_interpreterStack.pop_front();
		return value;
	}
	inline uint32_t TTFInstructionExecutor::GetNextStackUInt32() {
		if (_interpreterStack.empty())
			THROW("Can't retrieve a value of an empty stack");
		int32_t value = _interpreterStack.front();
		_interpreterStack.pop_front();
		return static_cast<uint32_t>(value);
	}
	inline F26Dot6 TTFInstructionExecutor::GetNextStackF26Dot6() {
		if (_interpreterStack.empty())
			THROW("Can't retrieve a value of an empty stack");
		int32_t value = _interpreterStack.front();
		_interpreterStack.pop_front();
		return value;
	}

	// Point getters and setters
	inline Utils::Vec2<F26Dot6> TTFInstructionExecutor::GetPoint(const uint32_t p, const uint32_t zone) {
		if (zone == 1) { if (p >= 0 && p < _points.size()) return _points[p]; else THROW("Invalid point, outside of range of valid points"); }
		else if (zone == 0) { if (p >= 0 && p < _state._twilightPoints.size()) return _state._twilightPoints[p]; else THROW("Invalid point, outside of range of valid twilight points"); }
		else THROW("Invalid zone");
	}
	inline Utils::Vec2<F26Dot6> TTFInstructionExecutor::GetOriginalPoint(const uint32_t p, const uint32_t zone) {
		if (zone == 1) { if (p >= 0 && p < _points.size()) return _originalPoints[p]; else THROW("Invalid point, outside of range of valid points"); }
		else if (zone == 0) { return _state._originalTwilightPoints[p]; }
		else THROW("Invalid zone");
	}
	inline void TTFInstructionExecutor::SetPoint(const uint32_t p, const uint32_t zone, Utils::Vec2<F26Dot6> value) {
		if (zone == 1) {
			if (p >= 0 && p < _points.size()) {
				_points[p] = value;
				SetPointTouched(p, zone);
			}
			else THROW("Invalid point, outside of range of valid points");
		}
		else if (zone == 0) {
			if (p > _state._twilightPoints.size())
				_state._twilightPoints.resize(p+1);
			_state._twilightPoints[p] = value;
		}
		else THROW("Invalid zone");
	}
	inline void TTFInstructionExecutor::AddToPoint(const uint32_t p, const uint32_t zone, Utils::Vec2<F26Dot6> additive) {
		AddToPointWithoutTouching(p, zone, additive);
		SetPointTouched(p, zone);
	}
	inline void TTFInstructionExecutor::AddToPointWithoutTouching(const uint32_t p, const uint32_t zone, Utils::Vec2<F26Dot6> additive) {
		if (zone == 1) {
			if (p >= 0 && p < _points.size()) { _points[p] += additive; }
			else THROW("Invalid point, outside of range of valid points");
		}
		else if (zone == 0) {
			if (p < 0 || p > _state._twilightPoints.size()) THROW("Invalid point, outside of range of valid twilight points");
			_state._twilightPoints[p] += additive;
		}
		else THROW("Invalid zone");
	}
	inline void TTFInstructionExecutor::SetPointTouched(const uint32_t p, const uint32_t zone) {
		SetPointTouched(p, zone, _state._freedomVector.x != 0, _state._freedomVector.y != 0);
	}
	inline void TTFInstructionExecutor::SetPointTouched(const uint32_t p, const uint32_t zone, const bool x, const bool y) {
		if (zone == 1 && p < _flags.size()) {
			if (x) _flags[p] |= GLYF_TOUCHED_X;
			if (y) _flags[p] |= GLYF_TOUCHED_Y;
		}
	}
	inline void TTFInstructionExecutor::SetPointUntouched(const uint32_t p, const uint32_t zone) {
		SetPointUntouched(p, zone, _state._freedomVector.x != 0, _state._freedomVector.y != 0);
	}
	inline void TTFInstructionExecutor::SetPointUntouched(const uint32_t p, const uint32_t zone, const bool x, const bool y) {
		if (zone == 1 && p < _flags.size()) {
			if (x) _flags[p] &= (0xFF ^ GLYF_TOUCHED_X);
			if (y) _flags[p] &= (0xFF ^ GLYF_TOUCHED_Y);
		}
	}
	inline void TTFInstructionExecutor::MovePointToProjectedValue(const uint32_t p, const uint32_t zone, const F26Dot6 value) {
		if (_state._freedomVector * _state._projectionVector == 0)
			THROW("Freedom and projection vector can't be orthoganol when moving");

		Utils::Vec2<F26Dot6> point = GetPoint(p, zone);
		float multiplicationFactor = (value - ProjectPoint(point)) / (_state._freedomVector * _state._projectionVector);
		AddToPoint(p, zone, _state._freedomVector * multiplicationFactor);
	}
	inline F26Dot6 TTFInstructionExecutor::ProjectPoint(const Utils::Vec2<F26Dot6> point) {
		return (F26Dot6)(_state._projectionVector * point);
	}
	inline F26Dot6 TTFInstructionExecutor::DualProjectPoint(const Utils::Vec2<F26Dot6> point) {
		return (F26Dot6)(_state._dualProjectionVector * point);
	}
	// CVT
	inline F26Dot6 TTFInstructionExecutor::GetCVTValue(const uint32_t n) {
		if (n >= _controlValues.size()) THROW("Illegal CVT index");
		return _controlValues[n];
	}
	inline void TTFInstructionExecutor::SetCVTValue(const uint32_t n, const F26Dot6 value) {
		if (n >= _controlValues.size()) THROW("Illegal CVT index");
		_controlValues[n] = value;
	}
	// Utils
	inline Utils::Vec2<F26Dot6> TTFInstructionExecutor::RoundToGrid(const Utils::Vec2<F26Dot6> p) {
		return Utils::Vec2<F26Dot6>((p.x + 32) & (0xFFFFFFFF ^ 0x3F), (p.y + 32) & (0xFFFFFFFF ^ 0x3F));
	}

	// ---------------------------------------------------
	// Actual logic
	// ---------------------------------------------------

	void TTFInstructionExecutor::ExecuteStack() {
		_currentInstruction = 0;
		while (_currentInstruction < _instructionStream.size()) {
			ExecuteCommand(GetNextInstructionByte());
		}
		_instructionStream.clear();
		if (!_interpreterStack.empty()) ERROR("TTF interpreter stack isn't empty");
		_interpreterStack.clear();
	}

	void TTFInstructionExecutor::ExecuteCommand(const uint8_t command) {
		switch (command) {
		// Push commands
		case 0x40: PushNBytes(); break;
		case 0x41: PushNWords(); break;
		case 0xB0: PushBytes(1); break;
		case 0xB1: PushBytes(2); break;
		case 0xB2: PushBytes(3); break;
		case 0xB3: PushBytes(4); break;
		case 0xB4: PushBytes(5); break;
		case 0xB5: PushBytes(6); break;
		case 0xB6: PushBytes(7); break;
		case 0xB7: PushBytes(8); break;
		case 0xB8: PushWords(1); break;
		case 0xB9: PushWords(2); break;
		case 0xBA: PushWords(3); break;
		case 0xBB: PushWords(4); break;
		case 0xBC: PushWords(5); break;
		case 0xBD: PushWords(6); break;
		case 0xBE: PushWords(7); break;
		case 0xBF: PushWords(8); break;
		// Store commands
		case 0x43: ReadStore(); break;
		case 0x42: WriteStore(); break;
		// CVT commands
		case 0x44: WriteCVTInPixels(); break;
		case 0x70: WriteCVTInFDU(); break;
 		case 0x45: ReadCVT(); break;
		// Vectors commands
		case 0x00: SetFreedomAndProjectionVectorToAxis(false); break;
		case 0x01: SetFreedomAndProjectionVectorToAxis(true); break;
		case 0x02: SetProjectionVectorToAxis(false); break;
		case 0x03: SetProjectionVectorToAxis(true); break;
		case 0x04: SetFreedomVectorToAxis(false); break;
		case 0x05: SetFreedomVectorToAxis(true); break;
		case 0x06: SetProjectionVectorToLine(false); break;
		case 0x07: SetProjectionVectorToLine(true); break;
		case 0x08: SetFreedomVectorToLine(false); break;
		case 0x09: SetFreedomVectorToLine(true); break;
		case 0x0E: SetFreedomVectorToProjectionVector(); break;
		case 0x86: SetDualProjectionVectorToLine(false); break;
		case 0x87: SetDualProjectionVectorToLine(true); break;
		case 0x0A: SetProjectionVectorFromStack(); break;
		case 0x0B: SetFreedomVectorFromStack(); break;
		case 0x0C: GetProjectionVector(); break;
		case 0x0D: GetFreedomVector(); break;
		// Reference points
		case 0x10: SetReferencePoint0(); break;
		case 0x11: SetReferencePoint1(); break;
		case 0x12: SetReferencePoint2(); break;
		// Zone pointers
		case 0x13: SetZonePointer0(); break;
		case 0x14: SetZonePointer1(); break;
		case 0x15: SetZonePointer2(); break;
		case 0x16: SetZonePointerS(); break;
		// Rounding commands
		case 0x19: RoundToHalfGrid(); break;
		case 0x18: RoundToGrid(); break;
		case 0x3D: RoundToDoubleGrid(); break;
		case 0x7D: RoundDownToGrid(); break;
		case 0x7C: RoundUpToGrid(); break;
		case 0x7A: RoundNone(); break;
		case 0x76: RoundSuper(); break;
		case 0x77: RoundSuper45Degree(); break;
		// Miscellaneous graphic state commands
		case 0x17: SetLoopVariable(); break;
		case 0x1A: SetMinimumDistance(); break;
		case 0x8E: SetInstructionExecutionControl(); break;
		case 0x85: SetScanConversionControl(); break;
		case 0x8D: SetScanType(); break;
		case 0x1D: SetCVTCutIn(); break;
		case 0x1E: SetSingleWidthCutIn(); break;
		case 0x1F: SetSingleWidth(); break;
		case 0x4D: SetAutoFlipOn(); break;
		case 0x4E: SetAutoFlipOff(); break;
		case 0x5E: SetDeltaBase(); break;
		case 0x5F: SetDeltaShift(); break;
		// Reading and writing data
		case 0x46: GetCoordinateProjectedOnProjectionVector(false); break;
		case 0x47: GetCoordinateProjectedOnProjectionVector(true); break;
		case 0x48: SetCoordinateFromStackUsingGraphicsVectors(); break;
		case 0x49: MeasureDistance(false); break;
		case 0x4A: MeasureDistance(true); break;
		case 0x4B: MeasurePixelsPerEM(); break;
		case 0x4C: MeasurePointSize(); break;
		// Outline managing
		case 0x80: FlipPoint(); break;
		case 0x81: FlipRangeOn(); break;
		case 0x82: FlipRangeOff(); break;
		case 0x32: ShiftPointByLastPoint(false); break;
		case 0x33: ShiftPointByLastPoint(true); break;
		case 0x34: ShiftContourByLastPoint(false); break;
		case 0x35: ShiftContourByLastPoint(true); break;
		case 0x36: ShiftZoneByLastPoint(false); break;
		case 0x37: ShiftZoneByLastPoint(true); break;
		case 0x38: ShiftPointByPixelAmount(); break;
		case 0x3A: MoveStackIndirectRelativePoint(false); break;
		case 0x3B: MoveStackIndirectRelativePoint(true); break;
		case 0x2E: MoveDirectAbsolutePoint(false); break;
		case 0x2F: MoveDirectAbsolutePoint(true); break;
		case 0x3E: MoveIndirectAbsolutePoint(false); break;
		case 0x3F: MoveIndirectAbsolutePoint(true); break;

		case 0xC0: MoveDirectRelativePoint(false, false, false, 0b00); break;
		case 0xC1: MoveDirectRelativePoint(false, false, false, 0b01); break;
		case 0xC2: MoveDirectRelativePoint(false, false, false, 0b10); break;
		case 0xC3: MoveDirectRelativePoint(false, false, false, 0b11); break;
		case 0xC4: MoveDirectRelativePoint(false, false, true, 0b00); break;
		case 0xC5: MoveDirectRelativePoint(false, false, true, 0b01); break;
		case 0xC6: MoveDirectRelativePoint(false, false, true, 0b10); break;
		case 0xC7: MoveDirectRelativePoint(false, false, true, 0b11); break;

		case 0xC8: MoveDirectRelativePoint(false, true, false, 0b00); break;
		case 0xC9: MoveDirectRelativePoint(false, true, false, 0b01); break;
		case 0xCA: MoveDirectRelativePoint(false, true, false, 0b10); break;
		case 0xCB: MoveDirectRelativePoint(false, true, false, 0b11); break;
		case 0xCC: MoveDirectRelativePoint(false, true, true, 0b00); break;
		case 0xCD: MoveDirectRelativePoint(false, true, true, 0b01); break;
		case 0xCE: MoveDirectRelativePoint(false, true, true, 0b10); break;
		case 0xCF: MoveDirectRelativePoint(false, true, true, 0b11); break;

		case 0xD0: MoveDirectRelativePoint(true, false, false, 0b00); break;
		case 0xD1: MoveDirectRelativePoint(true, false, false, 0b01); break;
		case 0xD2: MoveDirectRelativePoint(true, false, false, 0b10); break;
		case 0xD3: MoveDirectRelativePoint(true, false, false, 0b11); break;
		case 0xD4: MoveDirectRelativePoint(true, false, true, 0b00); break;
		case 0xD5: MoveDirectRelativePoint(true, false, true, 0b01); break;
		case 0xD6: MoveDirectRelativePoint(true, false, true, 0b10); break;
		case 0xD7: MoveDirectRelativePoint(true, false, true, 0b11); break;

		case 0xD8: MoveDirectRelativePoint(true, true, false, 0b00); break;
		case 0xD9: MoveDirectRelativePoint(true, true, false, 0b01); break;
		case 0xDA: MoveDirectRelativePoint(true, true, false, 0b10); break;
		case 0xDB: MoveDirectRelativePoint(true, true, false, 0b11); break;
		case 0xDC: MoveDirectRelativePoint(true, true, true, 0b00); break;
		case 0xDD: MoveDirectRelativePoint(true, true, true, 0b01); break;
		case 0xDE: MoveDirectRelativePoint(true, true, true, 0b10); break;
		case 0xDF: MoveDirectRelativePoint(true, true, true, 0b11); break;

		case 0xE0: MoveIndirectRelativePoint(false, false, false, 0b00); break;
		case 0xE1: MoveIndirectRelativePoint(false, false, false, 0b01); break;
		case 0xE2: MoveIndirectRelativePoint(false, false, false, 0b10); break;
		case 0xE3: MoveIndirectRelativePoint(false, false, false, 0b11); break;
		case 0xE4: MoveIndirectRelativePoint(false, false, true, 0b00); break;
		case 0xE5: MoveIndirectRelativePoint(false, false, true, 0b01); break;
		case 0xE6: MoveIndirectRelativePoint(false, false, true, 0b10); break;
		case 0xE7: MoveIndirectRelativePoint(false, false, true, 0b11); break;

		case 0xE8: MoveIndirectRelativePoint(false, true, false, 0b00); break;
		case 0xE9: MoveIndirectRelativePoint(false, true, false, 0b01); break;
		case 0xEA: MoveIndirectRelativePoint(false, true, false, 0b10); break;
		case 0xEB: MoveIndirectRelativePoint(false, true, false, 0b11); break;
		case 0xEC: MoveIndirectRelativePoint(false, true, true, 0b00); break;
		case 0xED: MoveIndirectRelativePoint(false, true, true, 0b01); break;
		case 0xEE: MoveIndirectRelativePoint(false, true, true, 0b10); break;
		case 0xEF: MoveIndirectRelativePoint(false, true, true, 0b11); break;

		case 0xF0: MoveIndirectRelativePoint(true, false, false, 0b00); break;
		case 0xF1: MoveIndirectRelativePoint(true, false, false, 0b01); break;
		case 0xF2: MoveIndirectRelativePoint(true, false, false, 0b10); break;
		case 0xF3: MoveIndirectRelativePoint(true, false, false, 0b11); break;
		case 0xF4: MoveIndirectRelativePoint(true, false, true, 0b00); break;
		case 0xF5: MoveIndirectRelativePoint(true, false, true, 0b01); break;
		case 0xF6: MoveIndirectRelativePoint(true, false, true, 0b10); break;
		case 0xF7: MoveIndirectRelativePoint(true, false, true, 0b11); break;
		
		case 0xF8: MoveIndirectRelativePoint(true, true, false, 0b00); break;
		case 0xF9: MoveIndirectRelativePoint(true, true, false, 0b01); break;
		case 0xFA: MoveIndirectRelativePoint(true, true, false, 0b10); break;
		case 0xFB: MoveIndirectRelativePoint(true, true, false, 0b11); break;
		case 0xFC: MoveIndirectRelativePoint(true, true, true, 0b00); break;
		case 0xFD: MoveIndirectRelativePoint(true, true, true, 0b01); break;
		case 0xFE: MoveIndirectRelativePoint(true, true, true, 0b10); break;
		case 0xFF: MoveIndirectRelativePoint(true, true, true, 0b11); break;

		case 0x3C: AlignRelativePoint(); break;
		case 0x0F: MovePointPToIntersection(); break;
		case 0x27: AlignPoints(); break;
		case 0x39: InterpolatePointByLastRelativeStretch(); break;
		case 0x29: UntouchPoint(); break;
		case 0x30: InterpolateUntouchedPointsThroughTheOutline(false); break;
		case 0x31: InterpolateUntouchedPointsThroughTheOutline(true); break;
		// Exceptions
		case 0x5D: DeltaExceptionP1(); break;
		case 0x71: DeltaExceptionP2(); break;
		case 0x72: DeltaExceptionP3(); break;
		case 0x73: DeltaExceptionC1(); break;
		case 0x74: DeltaExceptionC2(); break;
		case 0x75: DeltaExceptionC3(); break;
		// Managing the stack
		case 0x20: DuplicateTop(); break;
		case 0x21: PopTop(); break;
		case 0x22: ClearStack(); break;
		case 0x23: SwapTop(); break;
		case 0x24: ReturnDepth(); break;
		case 0x25: CopyIndexedToTop(); break;
		case 0x26: MoveIndexedToTop(); break;
		case 0x8a: RollTopThree(); break;
		// Flow control
		case 0x58: IfTest(); break;
		case 0x1B: Else(); break;
		case 0x59: EndIf(); break;
		case 0x78: JumpRelativeOnTrue(); break;
		case 0x1C: Jump(); break;
		case 0x79: JumpRelativeOnFalse(); break;
		// Logical functions
		case 0x50: LessThan(); break;
		case 0x51: LessThanOrEqual(); break;
		case 0x52: GreaterThan(); break;
		case 0x53: GreaterThanOrEqual(); break;
		case 0x54: Equal(); break;
		case 0x55: NotEqual(); break;
		case 0x56: Odd(); break;
		case 0x57: Even(); break;
		case 0x5A: LogicalAnd(); break;
		case 0x5B: LogicalOr(); break;
		case 0x5C: LogicalNot(); break;
		// Math functions
		case 0x60: Add(); break;
		case 0x61: Subtract(); break;
		case 0x62: Divide(); break;
		case 0x63: Multiply(); break;
		case 0x64: AbsoluteValue(); break;
		case 0x65: Negate(); break;
		case 0x66: Floor(); break;
		case 0x67: Ceiling(); break;
		case 0x8B: MaximumTop2(); break;
		case 0x8C: MinimumTop2(); break;
		// Round
		case 0x68: RoundInstruction(0); break;
		case 0x69: RoundInstruction(1); break;
		case 0x6A: RoundInstruction(2); break;
		case 0x6B: RoundInstruction(3); break;
		case 0x6C: NoRound(0); break;
		case 0x6D: NoRound(1); break;
		case 0x6E: NoRound(2); break;
		case 0x6F: NoRound(3); break;
		// Defining functions
		case 0x2C: DefineFunction(); break;
		case 0x2D: EndFunctionDefinition(); break;
		case 0x2B: CallFunction(); break;
		case 0x2A: LoopAndCallFunction(); break;
		case 0x89: InstructionDefinition(); break;
		// Debugging
		case 0x4F: Debug(); break;
		// Miscellaneous
		case 0x88: GetInformation(); break;
		// case 0x91: GetVariation(); break;

		default:
			THROW("Unkown TTF instruction = " + std::to_string(command));
		}
	}

	// ===================================================
	// Commands
	// ===================================================

	// Push commands
	void TTFInstructionExecutor::PushNBytes() {
		uint8_t numBytes = GetNextInstructionByte();
		for (uint8_t i = numBytes; i > 0; i--) {
			PushStack(GetNextInstructionByte());
		}
	}
	void TTFInstructionExecutor::PushNWords() {
		uint8_t numWords = GetNextInstructionByte();
		for (uint8_t i = numWords; i > 0; i--) {
			PushStack(GetNextInstructionPaddedWord());
		}
	}
	void TTFInstructionExecutor::PushBytes(const uint8_t num) {
		for (uint8_t i = num; i > 0; i--) {
			PushStack(GetNextInstructionByte());
		}
	}
	void TTFInstructionExecutor::PushWords(const uint8_t num) {
		for (uint8_t i = num; i > 0; i--) {
			PushStack(GetNextInstructionPaddedWord());
		}
	}

	// Store commands
	void TTFInstructionExecutor::ReadStore() {
		uint32_t location = GetNextStackUInt32();
		if (location >= _storageArea.size())
			THROW("Reading invalid value");
		PushStack(_storageArea[location]);
	}
	void TTFInstructionExecutor::WriteStore() {
		uint32_t value = GetNextStackUInt32();
		uint32_t location = GetNextStackUInt32();
		if (location >= _storageArea.size())
			_storageArea.resize(location + 1);
		_storageArea[location] = value;
	}

	// CVT commands
	void TTFInstructionExecutor::WriteCVTInPixels() {
		uint32_t value = GetNextStackUInt32();
		uint32_t location = GetNextStackUInt32();
		if (location >= _controlValues.size())
			_controlValues.resize(location + 1);
		_controlValues[location] = value;
	}
	void TTFInstructionExecutor::WriteCVTInFDU() {
		uint32_t value = GetNextStackUInt32();
		uint32_t location = GetNextStackUInt32();
		if (location >= _controlValues.size())
			_controlValues.resize(location + 1);
		_controlValues[location] = ToF26Dot6(value * _state._pointScale);
	}
	void TTFInstructionExecutor::ReadCVT() {
		uint32_t location = GetNextStackUInt32();
		if (location >= _controlValues.size())
			THROW("Reading invalid CVT value");
		PushStack(_controlValues[location]);
	}

	// Vectors commands
	void TTFInstructionExecutor::SetFreedomAndProjectionVectorToAxis(const bool xOrY) {
		if (xOrY) { // Set to the X-axis
			_state._projectionVector.x = 1;
			_state._projectionVector.y = 0;
			_state._freedomVector.x = 1;
			_state._freedomVector.y = 0;
		} else {    // Set to the Y-axis
			_state._projectionVector.x = 0;
			_state._projectionVector.y = 1;
			_state._freedomVector.x = 0;
			_state._freedomVector.y = 1;
		}
		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetProjectionVectorToAxis(const bool xOrY) {
		if (xOrY) { // Set to the X-axis
			_state._projectionVector.x = 1;
			_state._projectionVector.y = 0;
		}
		else {    // Set to the Y-axis
			_state._projectionVector.x = 0;
			_state._projectionVector.y = 1;
		}
		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetFreedomVectorToAxis(const bool xOrY) {
		if (xOrY) { // Set to the X-axis
			_state._freedomVector.x = 1;
			_state._freedomVector.y = 0;
		}
		else {    // Set to the Y-axis
			_state._freedomVector.x = 0;
			_state._freedomVector.y = 1;
		}
		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetProjectionVectorToLine(const bool perpendicular) {
		uint32_t p1 = GetNextStackUInt32();
		uint32_t p2 = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> point1 = GetPoint(p1, _state._zonePointer2);
		Utils::Vec2<F26Dot6> point2 = GetPoint(p2, _state._zonePointer1);

		_state._projectionVector = Utils::Vec2<F26Dot6>(point2 - point1);
		_state._projectionVector = _state._projectionVector.normalized();

		if (perpendicular)
			_state._projectionVector = _state._projectionVector.rotatedNegative90Degree();

		if(_state._projectionVector.x == 0 && _state._projectionVector.y == 0)
			ERROR("Received points that are the same")

		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetFreedomVectorToLine(const bool perpendicular) {
		uint32_t p1 = GetNextStackUInt32();
		uint32_t p2 = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> point1 = GetPoint(p1, _state._zonePointer2);
		Utils::Vec2<F26Dot6> point2 = GetPoint(p2, _state._zonePointer1);

		_state._freedomVector = Utils::Vec2<F26Dot6>(point2 - point1);
		_state._freedomVector = _state._freedomVector.normalized();

		if (perpendicular)
			_state._freedomVector = _state._freedomVector.rotatedNegative90Degree();

		if (_state._freedomVector.x == 0 && _state._freedomVector.y == 0)
			ERROR("Received points that are the same")

		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetFreedomVectorToProjectionVector() {
		_state._freedomVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetDualProjectionVectorToLine(const bool perpendicular) {
		uint32_t p1 = GetNextStackUInt32();
		uint32_t p2 = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> originalPoint1 = GetOriginalPoint(p1, _state._zonePointer2);
		Utils::Vec2<F26Dot6> originalPoint2 = GetOriginalPoint(p2, _state._zonePointer1);

		_state._dualProjectionVector = Utils::Vec2<F26Dot6>(originalPoint2 - originalPoint1);
		_state._dualProjectionVector = _state._dualProjectionVector.normalized();

		if (perpendicular)
			_state._dualProjectionVector = _state._dualProjectionVector.rotatedNegative90Degree();

		if (_state._dualProjectionVector.x == 0 && _state._dualProjectionVector.y == 0)
			ERROR("Received points that are the same")

		Utils::Vec2<F26Dot6> point1 = GetPoint(p1, _state._zonePointer2);
		Utils::Vec2<F26Dot6> point2 = GetPoint(p2, _state._zonePointer1);

		_state._projectionVector = Utils::Vec2<F26Dot6>(point2 - point1);
		_state._projectionVector = _state._projectionVector.normalized();

		if (perpendicular)
			_state._projectionVector = _state._projectionVector.rotatedNegative90Degree();

		if (_state._projectionVector.x == 0 && _state._projectionVector.y == 0)
			ERROR("Received points that are the same")
	}
	void TTFInstructionExecutor::SetProjectionVectorFromStack() {
		uint32_t y = GetNextStackUInt32();
		uint32_t x = GetNextStackUInt32();
		_state._projectionVector.y = (y & 0x0000FFFF) / 16384.f;
		_state._projectionVector.x = (x & 0x0000FFFF) / 16384.f;

		_state._dualProjectionVector = _state._projectionVector;
	}
	void TTFInstructionExecutor::SetFreedomVectorFromStack() {
		uint32_t y = GetNextStackUInt32();
		uint32_t x = GetNextStackUInt32();
		_state._freedomVector.y = (y & 0x0000FFFF) / 16384.f;
		_state._freedomVector.x = (x & 0x0000FFFF) / 16384.f;
	}
	void TTFInstructionExecutor::GetProjectionVector() {
		PushStack((uint32_t)std::floor(_state._projectionVector.x * 16384.0));
		PushStack((uint32_t)std::floor(_state._projectionVector.y * 16384.0));
	}
	void TTFInstructionExecutor::GetFreedomVector() {
		PushStack((uint32_t)std::floor(_state._freedomVector.x * 16384.0));
		PushStack((uint32_t)std::floor(_state._freedomVector.y * 16384.0));
	}

	// Reference points
	void TTFInstructionExecutor::SetReferencePoint0() {
		_state._refrencePoint0 = GetNextStackUInt32();
	}
	void TTFInstructionExecutor::SetReferencePoint1() {
		_state._refrencePoint1 = GetNextStackUInt32();
	}
	void TTFInstructionExecutor::SetReferencePoint2() {
		_state._refrencePoint2 = GetNextStackUInt32();
	}

	// Zone pointers
	void TTFInstructionExecutor::SetZonePointer0() {
		_state._zonePointer0 = GetNextStackInt32();
		if (_state._zonePointer0 != 0 && _state._zonePointer0 != 1)
			THROW("Invalid zone given");
	}
	void TTFInstructionExecutor::SetZonePointer1() {
		_state._zonePointer1 = GetNextStackInt32();
		if (_state._zonePointer1 != 0 && _state._zonePointer1 != 1)
			THROW("Invalid zone given");
	}
	void TTFInstructionExecutor::SetZonePointer2() {
		_state._zonePointer2 = GetNextStackInt32();
		if (_state._zonePointer2 != 0 && _state._zonePointer2 != 1)
			THROW("Invalid zone given");
	}
	void TTFInstructionExecutor::SetZonePointerS() {
		_state._zonePointer0 = GetNextStackInt32();
		_state._zonePointer1 = _state._zonePointer0;
		_state._zonePointer2 = _state._zonePointer0;
		if (_state._zonePointer0 != 0 && _state._zonePointer0 != 1)
			THROW("Invalid zone given");
	}

	// Rounding commands
	void TTFInstructionExecutor::RoundToHalfGrid() {
		_state._roundingState = GraphicsState::RoundState::ToHalfGrid;
	}
	void TTFInstructionExecutor::RoundToGrid() {
		_state._roundingState = GraphicsState::RoundState::ToGrid;
	}
	void TTFInstructionExecutor::RoundToDoubleGrid() {
		_state._roundingState = GraphicsState::RoundState::ToDoubleGrid;
	}
	void TTFInstructionExecutor::RoundDownToGrid() {
		_state._roundingState = GraphicsState::RoundState::DownToGrid;
	}
	void TTFInstructionExecutor::RoundUpToGrid() {
		_state._roundingState = GraphicsState::RoundState::UpToGrid;
	}
	void TTFInstructionExecutor::RoundNone() {
		_state._roundingState = GraphicsState::RoundState::None;
	}
	void TTFInstructionExecutor::RoundSuper() {
		_state._roundingState = GraphicsState::RoundState::Super;
		SetSuperRound(GetNextStackUInt32(), 0x40);
	}
	void TTFInstructionExecutor::RoundSuper45Degree() {
		_state._roundingState = GraphicsState::RoundState::Super45Degree;
		SetSuperRound(GetNextStackUInt32(), (float)sqrt(2) / 2.f);
	}

	void TTFInstructionExecutor::SetSuperRound(uint32_t n, float gridPeriod) {
		switch ((n & 0b11000000) >> 6) {
		case 0: _state._superRoundPeriod = gridPeriod / 2.f; break;
		case 1: _state._superRoundPeriod = gridPeriod;		  break;
		case 2: _state._superRoundPeriod = gridPeriod * 2;	  break;
		case 3: THROW("Invalid period for super round"); break;
		}
		switch ((n & 0b00110000) >> 4) {
		case 0: _state._superRoundPhase = 0; break;
		case 1: _state._superRoundPhase = _state._superRoundPeriod / 4.f; break;
		case 2: _state._superRoundPhase = _state._superRoundPeriod / 2.f; break;
		case 3: _state._superRoundPhase = gridPeriod * (3/4.f); break;
		}
		switch (n & 0b00001111) {
		case 0:  _state._superRoundThreshold = _state._superRoundPeriod - 1;    break;
		case 1:  _state._superRoundThreshold = (-3/8.f) * _state._superRoundPeriod; break;
		case 2:  _state._superRoundThreshold = (-2/8.f) * _state._superRoundPeriod; break;
		case 3:  _state._superRoundThreshold = (-1/8.f) * _state._superRoundPeriod; break;
		case 4:  _state._superRoundThreshold = (0 /8.f) * _state._superRoundPeriod; break;
		case 5:  _state._superRoundThreshold = (1 /8.f) * _state._superRoundPeriod; break;
		case 6:  _state._superRoundThreshold = (2 /8.f) * _state._superRoundPeriod; break;
		case 7:  _state._superRoundThreshold = (3 /8.f) * _state._superRoundPeriod; break;
		case 8:  _state._superRoundThreshold = (4 /8.f) * _state._superRoundPeriod; break;
		case 9:  _state._superRoundThreshold = (5 /8.f) * _state._superRoundPeriod; break;
		case 10: _state._superRoundThreshold = (6 /8.f) * _state._superRoundPeriod; break;
		case 11: _state._superRoundThreshold = (7 /8.f) * _state._superRoundPeriod; break;
		case 12: _state._superRoundThreshold = (8 /8.f) * _state._superRoundPeriod; break;
		case 13: _state._superRoundThreshold = (9 /8.f) * _state._superRoundPeriod; break;
		case 14: _state._superRoundThreshold = (10/8.f) * _state._superRoundPeriod; break;
		case 15: _state._superRoundThreshold = (11/8.f) * _state._superRoundPeriod; break;
		}
	}

	// Miscellaneous graphic state commands
	void TTFInstructionExecutor::SetLoopVariable() {
		_state._loopVariable = GetNextStackUInt32();
	}
	void TTFInstructionExecutor::SetMinimumDistance() {
		_state._minimumDistance = GetNextStackF26Dot6();
	}
	void TTFInstructionExecutor::SetInstructionExecutionControl() {
		int32_t selectorFlag = GetNextStackInt32();
		uint32_t value = GetNextStackUInt32();
		THROW("SetInstructionExecutionControl not implemented yet");
	}
	void TTFInstructionExecutor::SetScanConversionControl() {
		int32_t flags = GetNextStackInt32();
		ERROR("SetScanConversionControl not implemented yet");
	}
	void TTFInstructionExecutor::SetScanType() {
		int32_t n = GetNextStackInt32();
		ERROR("SetScanType not implemented yet");
	}
	void TTFInstructionExecutor::SetCVTCutIn() {
		_state._CVTCutIn = GetNextStackF26Dot6();
	}
	void TTFInstructionExecutor::SetSingleWidthCutIn() {
		_state._singleWidthCutIn = GetNextStackF26Dot6();
	}
	void TTFInstructionExecutor::SetSingleWidth() {
		uint32_t singleWidth = GetNextStackUInt32();
		_state._singleWidth = ToF26Dot6(singleWidth * _state._pointScale);
	}
	void TTFInstructionExecutor::SetAutoFlipOn() {
		_state._autoFlip = true;	}
	void TTFInstructionExecutor::SetAutoFlipOff() {
		_state._autoFlip = false;
	}
	void TTFInstructionExecutor::SetDeltaBase() {
		_state._deltaBase = GetNextStackUInt32();
	}
	void TTFInstructionExecutor::SetDeltaShift() {
		_state._deltaShift = GetNextStackUInt32();
	}

	// Reading and writing data
	void TTFInstructionExecutor::GetCoordinateProjectedOnProjectionVector(const bool originalOutline) {
		uint32_t p = GetNextStackUInt32();
		F26Dot6 d = 0;
		if (originalOutline)
			d = DualProjectPoint(GetOriginalPoint(p, _state._zonePointer2));
		else
			d = ProjectPoint(GetPoint(p, _state._zonePointer2));
		PushStack((F26Dot6)d);
	}
	void TTFInstructionExecutor::SetCoordinateFromStackUsingGraphicsVectors() {
		F26Dot6 value = GetNextStackF26Dot6();
		uint32_t p = GetNextStackUInt32();

		MovePointToProjectedValue(p, _state._zonePointer2, value);

		// !UNDOCUMENTED! taken from the FreeType project
		if (_state._zonePointer2 == 0)
			_state._originalTwilightPoints[p] = _state._twilightPoints[p];
		// ----------------------------------------------

		
	}
	void TTFInstructionExecutor::MeasureDistance(const bool originalOutline) {
		uint32_t p1 = GetNextStackUInt32();
		uint32_t p2 = GetNextStackUInt32();
		
		// !UNDOCUMENTED! taken from the FreeType project
		// originalOutline may be inverted, still the case????
		// ----------------------------------------------
		if (originalOutline) {
			F26Dot6 d1 = DualProjectPoint(GetOriginalPoint(p1, _state._zonePointer1));
			F26Dot6 d2 = DualProjectPoint(GetOriginalPoint(p2, _state._zonePointer0));
			PushStack(d2 - d1);
		}
		else {
			F26Dot6 d1 = ProjectPoint(GetPoint(p1, _state._zonePointer1));
			F26Dot6 d2 = ProjectPoint(GetPoint(p2, _state._zonePointer0));
			PushStack(d2 - d1);
		}
	}
	void TTFInstructionExecutor::MeasurePixelsPerEM() {
		//WARNING("NEEDS CHECKING, PROJECTION VECTOR SHOULD BE TAKEN INTO ACCOUNT"); // ppem = pointSize * dpi / 72
		PushStack(_state._pointSize);
	}
	void TTFInstructionExecutor::MeasurePointSize() {
		PushStack(ToF26Dot6(_state._pointSize));
	}

	// Outline managing

	void TTFInstructionExecutor::FlipPoint() {
		if (_state._zonePointer0 != 1)
			THROW("This function can only be used on real points");

		for (uint32_t i = 0; i < _state._loopVariable; i++) {
			uint32_t p = GetNextStackUInt32();
			_flags[p] ^= GLYF_ON_CURVE_POINT;
		}
		_state._loopVariable = 1;
		
		
	}
	void TTFInstructionExecutor::FlipRangeOn() {
		uint32_t highpoint = GetNextStackUInt32();
		uint32_t lowpoint = GetNextStackUInt32();

		for (uint32_t i = lowpoint; i <= highpoint; i++) {
			_flags[i] |= GLYF_ON_CURVE_POINT;
		}
	}
	void TTFInstructionExecutor::FlipRangeOff() {
		uint32_t highpoint = GetNextStackUInt32();
		uint32_t lowpoint = GetNextStackUInt32();

		for (uint32_t i = lowpoint; i <= highpoint; i++) {
			_flags[i] &= (0xFF ^ GLYF_ON_CURVE_POINT);
		}
	}
	void TTFInstructionExecutor::ShiftPointByLastPoint(const bool a) {
		// Get the distance the refrence pointer was shifted
		F26Dot6 distance = 0;
		if (a) {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint1, _state._zonePointer0));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint1, _state._zonePointer0));

			distance = projection1 - projection2;
		}
		else {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint2, _state._zonePointer1));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint2, _state._zonePointer1));

			distance = projection1 - projection2;
		}
		// Shift p
		for (uint32_t i = 0; i < _state._loopVariable; i++) {
			uint32_t p = GetNextStackUInt32();
			AddToPoint(p, _state._zonePointer2, _state._freedomVector * distance);
		}
		_state._loopVariable = 1;
	}
	// !UNDOCUMENTED! taken from the FreeType project
	// The twilight zone contains 1 contour with all the points in it at contour = 0
	// ----------------------------------------------
	void TTFInstructionExecutor::ShiftContourByLastPoint(const bool a) {
		uint32_t contour = GetNextStackUInt32();
		if ((_state._zonePointer2 == 1 && contour >= _endpoints.size())
			|| (_state._zonePointer2 == 0 && contour != 0))
			THROW("Illegal contour");
		uint32_t pStart = contour == 0 ? 0 : _endpoints[contour - 1];
		uint32_t pEnd = _endpoints[contour];

		// Get the distance the refrence pointer was shifted
		F26Dot6 distance = 0;
		if (a) {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint1, _state._zonePointer0));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint1, _state._zonePointer0));

			distance = projection1 - projection2;
		}
		else {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint2, _state._zonePointer1));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint2, _state._zonePointer1));

			distance = projection1 - projection2;
		}
		// Shift the contour
		if (_state._zonePointer2 == 0) {
			for (uint32_t i = 0; i < _state._twilightPoints.size(); i++) {
				AddToPoint(i, 0, _state._freedomVector * distance);
			}
		}
		else {
			for (uint32_t i = pStart; i < pEnd; i++) {
				AddToPoint(i, 1, _state._freedomVector * distance);
			}
		}
	}
	// !UNDOCUMENTED! taken from the FreeType project
	// Doesn't shift the phantom points and doesn't touch the points
	// ----------------------------------------------
	void TTFInstructionExecutor::ShiftZoneByLastPoint(const bool a) {
		uint32_t zone = GetNextStackUInt32();

		// Get the distance the refrence pointer was shifted
		F26Dot6 distance = 0;
		if (a) {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint1, _state._zonePointer0));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint1, _state._zonePointer0));

			distance = projection1 - projection2;
		}
		else {
			F26Dot6 projection1 = ProjectPoint(GetPoint(_state._refrencePoint2, _state._zonePointer1));
			F26Dot6 projection2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint2, _state._zonePointer1));

			distance = projection1 - projection2;
		}
		// Shift the whole zone
		if (zone == 0) {
			for (uint32_t i = 0; i < _state._twilightPoints.size(); i++) {
				AddToPointWithoutTouching(i, 0, _state._freedomVector * distance);
			}
		}
		else {
			for (uint32_t i = 0; i < _points.size() - 4; i++) {
				AddToPointWithoutTouching(i, 1, _state._freedomVector * distance);
			}
		}
	}
	void TTFInstructionExecutor::ShiftPointByPixelAmount() {
		F26Dot6 amount = GetNextStackF26Dot6();
		for (uint32_t i = 0; i < _state._loopVariable; i++) {
			uint32_t p = GetNextStackUInt32();
			AddToPoint(p, _state._zonePointer2, _state._freedomVector * amount);
		}
	}
	void TTFInstructionExecutor::MoveStackIndirectRelativePoint(const bool setRP0) {
		F26Dot6 d = GetNextStackF26Dot6();
		uint32_t p = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> rPoint = GetPoint(_state._refrencePoint0, _state._zonePointer0);
		// !UNDOCUMENTED! taken from the FreeType project
		if (_state._zonePointer1 == 0) {
			_state._originalTwilightPoints[p] = rPoint;
			_state._originalTwilightPoints[p] += _state._freedomVector * d;
			_state._twilightPoints[p] = _state._originalTwilightPoints[p];
		}
		// ----------------------------------------------
		MovePointToProjectedValue(p, _state._zonePointer1, ProjectPoint(rPoint) + d);

		_state._refrencePoint1 = _state._refrencePoint0;
		_state._refrencePoint2 = p;
		if (setRP0)
			_state._refrencePoint0 = p;
	}
	void TTFInstructionExecutor::MoveDirectAbsolutePoint(const bool applyRounding) {
		uint32_t p = GetNextStackUInt32();
		SetPointTouched(p, _state._zonePointer0);

		if (applyRounding) {
			Utils::Vec2<F26Dot6> point = GetPoint(p, _state._zonePointer0);
			F26Dot6 rounded = Round(ProjectPoint(point));
			MovePointToProjectedValue(p, _state._zonePointer0, rounded);
		}
		_state._refrencePoint0 = _state._refrencePoint1 = p;
	}
	void TTFInstructionExecutor::MoveIndirectAbsolutePoint(const bool applyRounding) {
		uint32_t n = GetNextStackUInt32();
		uint32_t p = GetNextStackUInt32();

		F26Dot6 position = GetCVTValue(n);

		// !UNDOCUMENTED! taken from the FreeType project
		if (_state._zonePointer0 == 0) { // If in twilight zone
			if (p >= _state._twilightPoints.size())
				THROW("Illegal index of a twilight point");
			_state._originalTwilightPoints[p] = _state._freedomVector * position;
			if (applyRounding)
				position = Round(position);
			MovePointToProjectedValue(p, 0, position);
			_state._refrencePoint0 = _state._refrencePoint1 = p;
			return;
		}
		// ----------------------------------------------

		if (applyRounding) {
			Utils::Vec2<F26Dot6> point = GetPoint(p, 1);
			if (std::abs(ProjectPoint(point) - position) > _state._CVTCutIn)
				position = Round(ProjectPoint(point));
			else
				position = Round(position);
		}
		MovePointToProjectedValue(p, 1, position);
		_state._refrencePoint0 = _state._refrencePoint1 = p;
	}
	void TTFInstructionExecutor::MoveDirectRelativePoint(const bool setRP0, const bool minimumDistance, const bool applyRounding, const uint8_t distanceType) {
		uint32_t p = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> originalPoint = GetOriginalPoint(p, _state._zonePointer1);
		Utils::Vec2<F26Dot6> originalRefrencePoint = GetOriginalPoint(_state._refrencePoint0, _state._zonePointer0);

		F26Dot6 originalDistance = DualProjectPoint(originalPoint) - DualProjectPoint(originalRefrencePoint);

		if (std::abs(_state._singleWidth - originalDistance) < _state._singleWidthCutIn)
			originalDistance = _state._singleWidth;

		if (applyRounding) {
			originalDistance = Round(originalDistance);
			if (minimumDistance && abs(originalDistance) < _state._minimumDistance)
				originalDistance = originalDistance > 0 ? _state._minimumDistance : _state._minimumDistance * -1;
		}

		Utils::Vec2<F26Dot6> refrencePoint = GetPoint(_state._refrencePoint0, _state._zonePointer0);
		F26Dot6 projected = ProjectPoint(refrencePoint);
		F26Dot6 hehe = ProjectPoint(GetPoint(p, _state._zonePointer1));
		MovePointToProjectedValue(p, _state._zonePointer1, ProjectPoint(refrencePoint) + originalDistance);

		_state._refrencePoint1 = _state._refrencePoint0;
		_state._refrencePoint2 = p;
		if (setRP0)
			_state._refrencePoint0 = p;
	}
	void TTFInstructionExecutor::MoveIndirectRelativePoint(const bool setRP0, const bool minimumDistance, const bool applyRounding, const uint8_t distanceType) {
		uint32_t n = GetNextStackUInt32();
		uint32_t p = GetNextStackUInt32();

		if (p == 11)
			p = 11;

		F26Dot6 distance = 0;
		// !UNDOCUMENTED! taken from the FreeType project
		if (n == -1)
			distance = 0;
		// ----------------------------------------------
		else
			distance = GetCVTValue(n);
		F26Dot6 cvtValue = distance;

		if (std::abs(_state._singleWidth - distance) < _state._singleWidthCutIn)
			distance = _state._singleWidth;

		// !UNDOCUMENTED! taken from the FreeType project
		if (_state._zonePointer1 == 0) {
			if (p >= _state._twilightPoints.size())
				THROW("Illegal index of a twilight point");
			_state._originalTwilightPoints[p] += _state._freedomVector * distance;
			_state._twilightPoints[p] = _state._originalTwilightPoints[p];
		}
		// ----------------------------------------------

		Utils::Vec2<F26Dot6> originalPoint = GetOriginalPoint(p, _state._zonePointer1);
		Utils::Vec2<F26Dot6> originalRefrencePoint = GetOriginalPoint(_state._refrencePoint0, _state._zonePointer0);
		F26Dot6 originalDistance = DualProjectPoint(originalPoint) - DualProjectPoint(originalRefrencePoint);

		if (_state._autoFlip) {
			if ((distance < 0) != (originalDistance < 0)) // If the sign changed
				distance = distance * -1;
		}

		if (applyRounding) {
			Utils::Vec2<F26Dot6> point = GetPoint(p, _state._zonePointer1);

			// !UNDOCUMENTED! taken from the FreeType project
			// Only perform cut in test if both point are from the same zone
			// ----------------------------------------------

			if (_state._zonePointer0 == _state._zonePointer1 && std::abs(originalDistance - distance) > _state._CVTCutIn)
				distance = Round(originalDistance);
			else
				distance = Round(distance);

			if (minimumDistance && abs(distance) < _state._minimumDistance)
				distance = distance > 0 ? _state._minimumDistance : _state._minimumDistance * -1;
		}

		Utils::Vec2<F26Dot6> refrencePoint = GetPoint(_state._refrencePoint0, _state._zonePointer0);
		MovePointToProjectedValue(p, _state._zonePointer1, ProjectPoint(refrencePoint) + distance);

		_state._refrencePoint1 = _state._refrencePoint0;
		_state._refrencePoint2 = p;
		if (setRP0)
			_state._refrencePoint0 = p;
	}
	void TTFInstructionExecutor::AlignRelativePoint() {
		Utils::Vec2<F26Dot6> refrencePos = GetPoint(_state._refrencePoint0, _state._zonePointer0);

		for (uint32_t i = 0; i < _state._loopVariable; i++) {
			uint32_t p = GetNextStackUInt32();
			MovePointToProjectedValue(p, _state._zonePointer1, ProjectPoint(refrencePos));
		}
		_state._loopVariable = 1;
	}
	void TTFInstructionExecutor::MovePointPToIntersection() {
		uint32_t b1 = GetNextStackUInt32();
		uint32_t b0 = GetNextStackUInt32();
		uint32_t a1 = GetNextStackUInt32();
		uint32_t a0 = GetNextStackUInt32();
		uint32_t p = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> BEnd = GetPoint(b1, _state._zonePointer0);
		Utils::Vec2<F26Dot6> BStart = GetPoint(b0, _state._zonePointer0);
		Utils::Vec2<F26Dot6> AEnd = GetPoint(a1, _state._zonePointer1);
		Utils::Vec2<F26Dot6> AStart = GetPoint(a0, _state._zonePointer1);

		float denominator = (float)((AStart.x - AEnd.x)*(BStart.y - BEnd.y) - (AStart.y - AEnd.y)*(BStart.x - BEnd.x));

		if (denominator == 0) {
			// They are parralel, set them to the middle
			F26Dot6 x = (AStart.x + AEnd.x + BStart.x + BEnd.x) / 4;
			F26Dot6 y = (AStart.y + AEnd.y + BStart.y + BEnd.y) / 4;
			SetPoint(p, _state._zonePointer2, Utils::Vec2<F26Dot6>(x, y));
			return;
		}

		float t =  ((AStart.x - BStart.x)*(BStart.y - BEnd.y) - (AStart.y - BStart.y)*(BStart.x - BEnd.x)) / denominator;
		float u = -((AStart.x - AEnd.x)*(AStart.y - BStart.y) - (AStart.y - AEnd.y)*(AStart.x - BStart.x)) / denominator;

		Utils::Vec2<F26Dot6> newPos = AStart + (AEnd - AStart) * t;

		SetPoint(p, _state._zonePointer2, newPos);
	}
	void TTFInstructionExecutor::AlignPoints() {
		uint32_t p1 = GetNextStackUInt32();
		uint32_t p2 = GetNextStackUInt32();

		Utils::Vec2<F26Dot6> P1 = GetPoint(p1, _state._zonePointer1);
		Utils::Vec2<F26Dot6> P2 = GetPoint(p2, _state._zonePointer0);

		F26Dot6 middle = (ProjectPoint(P1) + ProjectPoint(P2)) / 2;

		MovePointToProjectedValue(p1, _state._zonePointer1, middle);
		MovePointToProjectedValue(p2, _state._zonePointer0, middle);
	}
	void TTFInstructionExecutor::InterpolatePointByLastRelativeStretch() {
		F26Dot6 refrencePoint1 = ProjectPoint(GetPoint(_state._refrencePoint1, _state._zonePointer0));
		F26Dot6 refrencePoint2 = ProjectPoint(GetPoint(_state._refrencePoint2, _state._zonePointer1));
		F26Dot6 originalRefrencePoint1 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint1, _state._zonePointer0));
		F26Dot6 originalRefrencePoint2 = DualProjectPoint(GetOriginalPoint(_state._refrencePoint2, _state._zonePointer1));

		F26Dot6 ref1ToRef2 = refrencePoint2 - refrencePoint1;

		for (uint32_t i = 0; i < _state._loopVariable; i++) {
			uint32_t p = GetNextStackUInt32();
			F26Dot6 originalPoint = DualProjectPoint(GetOriginalPoint(p, _state._zonePointer2));

			F26Dot6 distancePToRef1 = originalPoint - originalRefrencePoint1;
			F26Dot6 distanceRef2ToP = originalRefrencePoint2 - originalPoint;

			if (p == 0)
				p = 0;

			MovePointToProjectedValue(p, _state._zonePointer2, refrencePoint1 + (F26Dot6)(ref1ToRef2 * (distancePToRef1 / ((double)distancePToRef1 + (double)distanceRef2ToP))));
		}
		_state._loopVariable = 1;
	}
	void TTFInstructionExecutor::UntouchPoint() {
		uint32_t p = GetNextStackUInt32();

		SetPointUntouched(p, _state._zonePointer0);
	}
	void TTFInstructionExecutor::InterpolateUntouchedPointsThroughTheOutline(const bool xOrY) {
		if (_state._zonePointer2 != 1)
			THROW("No zone other then zone 1 contains outlines");

		uint8_t flag = xOrY ? GLYF_TOUCHED_X : GLYF_TOUCHED_Y;

		uint32_t outlineStart = 0;
		for (uint32_t outlineEnd : _endpoints) {
			uint32_t firstTouched = outlineStart;
			while (!(_flags[firstTouched] & flag) && firstTouched < outlineEnd)
				firstTouched++;
			if (firstTouched >= outlineEnd)
				continue;

			uint32_t currentTouched = firstTouched;
			uint32_t currentPoint = firstTouched + 1;
			do {
				if (_flags[currentPoint] & flag) {
					F26Dot6 originalTouchedPos = xOrY ? _originalPoints[currentTouched].x : _originalPoints[currentTouched].y;
					F26Dot6 originalFoundPointPos = xOrY ? _originalPoints[currentPoint].x : _originalPoints[currentPoint].y;
					F26Dot6 currentTouchedPos = xOrY ? _points[currentTouched].x : _points[currentTouched].y;
					F26Dot6 currentFoundPointPos = xOrY ? _points[currentPoint].x : _points[currentPoint].y;
					for (
						uint32_t i = currentTouched + 1 > outlineEnd ? outlineStart : currentTouched + 1; 
						i != currentPoint; 
						i = i + 1 > outlineEnd ? outlineStart : i + 1
						) {
						F26Dot6 originalPointPos = xOrY ? _originalPoints[i].x : _originalPoints[i].y;
						F26Dot6 currentPointPos = xOrY ? _points[i].x : _points[i].y;
						F26Dot6 delta = 0;
						if (originalTouchedPos < originalPointPos && originalFoundPointPos > originalPointPos) {
							float partOfTotalToPoint = (originalPointPos - originalTouchedPos) / (float)(originalFoundPointPos - originalTouchedPos);// (distance currentTouched to i)/(distance currentTouched to currentPoint)
							delta = -(F26Dot6)(currentPointPos - (currentTouchedPos + partOfTotalToPoint * (currentFoundPointPos - currentTouchedPos)));
						}
						else if (originalFoundPointPos < originalPointPos && originalTouchedPos > originalPointPos) {
							float partOfTotalToPoint = (originalPointPos - originalFoundPointPos) / (float)(originalTouchedPos - originalFoundPointPos);// (distance currentPoint to i)/(distance currentPoint to currentTouched)
							delta = -(F26Dot6)(currentPointPos - (currentFoundPointPos + partOfTotalToPoint * (currentTouchedPos - currentFoundPointPos)));
						}
						else if ((originalTouchedPos < originalFoundPointPos && originalPointPos >= originalFoundPointPos)
							||   (originalTouchedPos > originalFoundPointPos && originalPointPos <= originalFoundPointPos)) {
							delta = currentFoundPointPos - originalFoundPointPos;
						}
						else {
							delta = currentTouchedPos - originalTouchedPos;
						}
						if (xOrY) _state._IUPDeltas[i].x = delta; else _state._IUPDeltas[i].y = delta;
					}
					if (xOrY) _state._IUPDeltas[currentTouched].x = 0; else _state._IUPDeltas[currentTouched].y = 0;
					if (xOrY) _state._IUPDeltas[currentPoint].x = 0; else _state._IUPDeltas[currentPoint].y = 0;

					currentTouched = currentPoint;
				}
				currentPoint = currentPoint + 1 > outlineEnd ? outlineStart : currentPoint + 1;
			} while (currentPoint != firstTouched + 1);

			outlineStart = outlineEnd + 1;
		}
	}

	// Exceptions
	void TTFInstructionExecutor::DeltaExceptionP1() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t p = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 pos = ProjectPoint(GetPoint(p, _state._zonePointer0)) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				MovePointToProjectedValue(p, _state._zonePointer0, pos);
			}
		}
	}
	void TTFInstructionExecutor::DeltaExceptionP2() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t p = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + 16 + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 pos = ProjectPoint(GetPoint(p, _state._zonePointer0)) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				MovePointToProjectedValue(p, _state._zonePointer0, pos);
			}
		}
	}
	void TTFInstructionExecutor::DeltaExceptionP3() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t p = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + 32 + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 pos = ProjectPoint(GetPoint(p, _state._zonePointer0)) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				MovePointToProjectedValue(p, _state._zonePointer0, pos);
			}
		}
	}
	void TTFInstructionExecutor::DeltaExceptionC1() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t entry = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 value = GetCVTValue(entry) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				SetCVTValue(entry, value);
			}
		}
	}
	void TTFInstructionExecutor::DeltaExceptionC2() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t entry = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + 16 + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 value = GetCVTValue(entry) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				SetCVTValue(entry, value);
			}
		}
	}
	void TTFInstructionExecutor::DeltaExceptionC3() {
		uint32_t n = GetNextStackUInt32();

		for (uint32_t i = 0; i < n; i++) {
			uint32_t entry = GetNextStackUInt32();
			int32_t arg = GetNextStackUInt32();
			uint32_t ppemTrigger = _state._deltaBase + 32 + ((arg & 0xF0) >> 4);
			if (_state._pixelsPerEM == ppemTrigger) {
				int steps = (arg & 0x0F) >= 8 ? (arg & 0x0F) - 7 : (arg & 0x0F) - 8;// Map the values from -8 to 8 while skipping 0
				F26Dot6 value = GetCVTValue(entry) + ToF26Dot6(steps / std::pow(2, _state._deltaShift));
				SetCVTValue(entry, value);
			}
		}
	}

	// Managing the stack
	void TTFInstructionExecutor::DuplicateTop() {
		PushStack(_interpreterStack.front());
	}
	void TTFInstructionExecutor::PopTop() {
		_interpreterStack.pop_front();
	}
	void TTFInstructionExecutor::ClearStack() {
		_interpreterStack.clear();
	}
	void TTFInstructionExecutor::SwapTop() {
		uint32_t first = GetNextStackUInt32();
		uint32_t second = GetNextStackUInt32();
		PushStack(first);
		PushStack(second);
	}
	void TTFInstructionExecutor::ReturnDepth() {
		PushStack((uint32_t)_interpreterStack.size());
	}
	void TTFInstructionExecutor::CopyIndexedToTop() {
		uint32_t index = GetNextStackUInt32();
		PushStack(_interpreterStack[index - 1]);
	}
	void TTFInstructionExecutor::MoveIndexedToTop() {
		uint32_t index = GetNextStackUInt32();
		uint32_t value = _interpreterStack[index - 1];
		_interpreterStack.erase(_interpreterStack.begin() + index - 1);
		PushStack(value);
	}
	void TTFInstructionExecutor::RollTopThree() {
		uint32_t a = GetNextStackUInt32();
		uint32_t b = GetNextStackUInt32();
		uint32_t c = GetNextStackUInt32();
		PushStack(b);
		PushStack(a);
		PushStack(c);
	}

	// Flow control
	void TTFInstructionExecutor::IfTest() {
		uint32_t condition = GetNextStackUInt32();
		if (condition) {
			// Continue executing until the else statement
			return;
		}
		else {
			// Find an else or an endif statement
			int nesting = 0;
			while (IsNextInstructionLeft()) {
				uint8_t nextInstruction = GetNextInstructionByte();
				// Skip the instructions taking values from the instruction stream
				if (nextInstruction >= 0xB0 && nextInstruction <= 0xB7) { JumpForwardAmountInstruction(nextInstruction - 0xB0 + 1); }
				else if (nextInstruction >= 0xB8 && nextInstruction <= 0xBF) { JumpForwardAmountInstruction((nextInstruction - 0xB8 + 1) * 2); }
				else if (nextInstruction == 0x40) { uint8_t amount = GetNextInstructionByte(); JumpForwardAmountInstruction(amount); }
				else if (nextInstruction == 0x41) { uint8_t amount = GetNextInstructionByte() * 2; JumpForwardAmountInstruction(amount); }
				else if (nextInstruction == 0x58) { // If
					nesting++;
				}
				else if (nextInstruction == 0x1B && nesting == 0) { // Else
					return;
				}
				else if (nextInstruction == 0x59) { // Endif
					if (nesting != 0) {
						nesting--;
						continue;
					}
					return;
				}
			}
		}
	}
	void TTFInstructionExecutor::Else() {
		// Skip forward to the endif statement
		int nesting = 0;
		while (IsNextInstructionLeft()) {
			uint8_t nextInstruction = GetNextInstructionByte();
			// Skip the instructions taking values from the instruction stream
			if (nextInstruction >= 0xB0 && nextInstruction <= 0xB7) { JumpForwardAmountInstruction(nextInstruction - 0xB0 + 1); }
			else if (nextInstruction >= 0xB8 && nextInstruction <= 0xBF) { JumpForwardAmountInstruction((nextInstruction - 0xB8 + 1) * 2); }
			else if (nextInstruction == 0x40) { uint8_t amount = GetNextInstructionByte(); JumpForwardAmountInstruction(amount); }
			else if (nextInstruction == 0x41) { uint8_t amount = GetNextInstructionByte() * 2; JumpForwardAmountInstruction(amount); }
			else if (nextInstruction == 0x58) { // If
				nesting++;
			}
			else if (nextInstruction == 0x59) { // Endif
				if (nesting != 0) {
					nesting--;
					continue;
				}
				return;
			}
		}
	}
	void TTFInstructionExecutor::EndIf() {
		// Ignore this statement
	}
	void TTFInstructionExecutor::JumpRelativeOnTrue() {
		uint32_t condition = GetNextStackUInt32();
		int32_t amountJump = GetNextStackInt32();
		if (amountJump == 0)
			THROW("Can't jump back to same jump instruction, this will cause a loop");
		if (condition)
			JumpForwardAmountInstruction(amountJump - 1);
	}
	void TTFInstructionExecutor::Jump() {
		int32_t amountJump = GetNextStackInt32();
		if (amountJump == 0)
			THROW("Can't jump back to same jump instruction, this will cause a loop");
		JumpForwardAmountInstruction(amountJump - 1);
	}
	void TTFInstructionExecutor::JumpRelativeOnFalse() {
		uint32_t condition = GetNextStackUInt32();
		int32_t amountJump = GetNextStackInt32();
		if (amountJump == 0)
			THROW("Can't jump back to same jump instruction, this will cause a loop");
		if (!condition)
			JumpForwardAmountInstruction(amountJump - 1);
	}

	// Logical functions
	void TTFInstructionExecutor::LessThan() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 < e2);
	}
	void TTFInstructionExecutor::LessThanOrEqual() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 <= e2);
	}
	void TTFInstructionExecutor::GreaterThan() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 > e2);
	}
	void TTFInstructionExecutor::GreaterThanOrEqual() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 >= e2);
	}
	void TTFInstructionExecutor::Equal() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 == e2);
	}
	void TTFInstructionExecutor::NotEqual() {
		uint32_t e2 = GetNextStackUInt32();
		uint32_t e1 = GetNextStackUInt32();

		PushStack(e1 != e2);
	}
	void TTFInstructionExecutor::Odd() {
		F26Dot6 e1 = GetNextStackF26Dot6();
		e1 = Round(e1);
		if (e1 % 0x40)
			PushStack(true);
		else
			PushStack(false);
	}
	void TTFInstructionExecutor::Even() {
		F26Dot6 e1 = GetNextStackF26Dot6();
		e1 = Round(e1);
		if (e1 % 0x40)
			PushStack(false);
		else
			PushStack(true);
	}
	void TTFInstructionExecutor::LogicalAnd() {
		uint32_t e1 = GetNextStackUInt32();
		uint32_t e2 = GetNextStackUInt32();

		PushStack(e1 && e2);
	}
	void TTFInstructionExecutor::LogicalOr() {
		uint32_t e1 = GetNextStackUInt32();
		uint32_t e2 = GetNextStackUInt32();

		PushStack(e1 || e2);
	}
	void TTFInstructionExecutor::LogicalNot() {
		uint32_t e = GetNextStackUInt32();

		PushStack(!e);
	}

	// Math functions
	void TTFInstructionExecutor::Add() {
		F26Dot6 n1 = GetNextStackF26Dot6();
		F26Dot6 n2 = GetNextStackF26Dot6();

		PushStack(n2 + n1);
	}
	void TTFInstructionExecutor::Subtract() {
		F26Dot6 n1 = GetNextStackF26Dot6();
		F26Dot6 n2 = GetNextStackF26Dot6();

		PushStack(n2 - n1);
	}
	void TTFInstructionExecutor::Divide() {
		F26Dot6 n1 = GetNextStackF26Dot6();
		F26Dot6 n2 = GetNextStackF26Dot6();

		int64_t devisor = n1;
		int64_t dividend = (int64_t)n2 * 0x40;

		PushStack((F26Dot6)(dividend / devisor));
	}
	void TTFInstructionExecutor::Multiply() {
		F26Dot6 n1 = GetNextStackF26Dot6();
		F26Dot6 n2 = GetNextStackF26Dot6();

		int64_t multiple = (int64_t)n1 * n2;

		PushStack((F26Dot6)(multiple / (0x40)));
	}
	void TTFInstructionExecutor::AbsoluteValue() {
		F26Dot6 n = GetNextStackF26Dot6();
		
		if (n < 0)
			PushStack(n * -1);
		else
			PushStack(n);
	}
	void TTFInstructionExecutor::Negate() {
		F26Dot6 n = GetNextStackF26Dot6();
		PushStack(n * -1);
	}
	void TTFInstructionExecutor::Floor() {
		F26Dot6 n = GetNextStackF26Dot6();
		if (!(n & 0x3F)) {
			PushStack(n);
		} else if (n > 0) {
			// Just remove the last bits
			PushStack(n & 0xFFFFFFC0);
		} else {
			// Remove last bits and remove 1
			PushStack((n & 0xFFFFFFC0) - 0x40);
		}
	}
	void TTFInstructionExecutor::Ceiling() {
		F26Dot6 n = GetNextStackF26Dot6();
		if (!(n & 0x3F)) {
			PushStack(n);
		} else if (n > 0) {
			// Remove the last bits and add 1
			PushStack((n & 0xFFFFFFC0) + 0x40);
		}
		else {
			// Remove last bits and remove 1
			PushStack(n & 0xFFFFFFC0);
		}
	}
	void TTFInstructionExecutor::MaximumTop2() {
		uint32_t e1 = GetNextStackUInt32();
		uint32_t e2 = GetNextStackUInt32();

		if(e1 > e2) 
			PushStack(e1);
		else
			PushStack(e2);
	}
	void TTFInstructionExecutor::MinimumTop2() {
		uint32_t e1 = GetNextStackUInt32();
		uint32_t e2 = GetNextStackUInt32();

		if (e1 < e2)
			PushStack(e1);
		else
			PushStack(e2);
	}

	// Round
	void TTFInstructionExecutor::RoundInstruction(const uint8_t ab) {
		F26Dot6 n1 = GetNextStackF26Dot6();

		PushStack(Round(n1));
	}
	void TTFInstructionExecutor::NoRound(const uint8_t ab) {
		// Just do nothing, we aren't compensating for engine characteristics
	}
	F26Dot6 TTFInstructionExecutor::Round(const F26Dot6 n, const F26Dot6 period, const F26Dot6 phase, const F26Dot6 threshold) {
		F26Dot6 value = n > 0 ? n : n*-1; // I seriously have no idea why this is neccesary but it is done by FreeType and makes things look a lot better
		value -= phase;
		value += threshold;
		if (period == 0x20)
			value &= (0xFFFFFFFF ^ 0x1F);
		else if (period == 0x40)
			value &= (0xFFFFFFFF ^ 0x3F);
		else if (period == 0x80)
			value &= (0xFFFFFFFF ^ 0x7F);
		else
			THROW("Illegal rounding period");
		value += phase;

		if (n < 0)
			value *= -1;

		if (n > 0 && value < 0) {
			value = phase;
			ERROR("Plase check rounding result")
		}
		else if (n < 0 && value > 0) {
			value = -phase;
			ERROR("Plase check rounding result")
		}

		return value;
	}
	F26Dot6 TTFInstructionExecutor::Round(const F26Dot6 n, const float period, const float phase, const float threshold) {
		float value = n / 64.0f;
		value -= phase;
		value += threshold;
		THROW("Not implemented trunctuating to period");
		value += phase;

		if (n > 0 && value < 0) {
			value = phase;
			ERROR("Plase check rounding result")
		}
		else if (n < 0 && value > 0) {
			value = -phase;
			ERROR("Plase check rounding result")
		}

		return (F26Dot6)(value * 0x40);
	}
	F26Dot6 TTFInstructionExecutor::Round(const F26Dot6 n) {
		switch (_state._roundingState) {
		case GraphicsState::RoundState::ToHalfGrid: return Round(n, 0x40, 0x20, 0x20);
		case GraphicsState::RoundState::ToGrid: return Round(n, 0x40, 0, 0x20);
		case GraphicsState::RoundState::ToDoubleGrid: return Round(n, 0x80, 0, 0x40);
		case GraphicsState::RoundState::DownToGrid: return Round(n, 0x40, 0, 0);
		case GraphicsState::RoundState::UpToGrid: return Round(n, 0x40, 0, 0x40);
		case GraphicsState::RoundState::None: return n;
		case GraphicsState::RoundState::Super: return Round(n, (F26Dot6)_state._superRoundPeriod, (F26Dot6)_state._superRoundPhase, (F26Dot6)_state._superRoundThreshold);
		case GraphicsState::RoundState::Super45Degree: return Round(n, _state._superRoundPeriod, _state._superRoundPhase, _state._superRoundThreshold);
		}
		THROW("Invalid TTF rounding state")
		return 0;
	}

	// Defining functions
	void TTFInstructionExecutor::DefineFunction() {
		uint32_t f = GetNextStackUInt32();
		size_t functionStart = _currentInstruction;
		size_t functionEnd = _currentInstruction + 1;
		while(functionEnd < _instructionStream.size()) {
			uint8_t nextInstruction = GetNextInstructionByte();
			// Skip the instructions taking values from the instruction stream
			if (nextInstruction >= 0xB0 && nextInstruction <= 0xB7) { JumpForwardAmountInstruction(nextInstruction - 0xB0 + 1); }
			else if (nextInstruction >= 0xB8 && nextInstruction <= 0xBF) { JumpForwardAmountInstruction((nextInstruction - 0xB8 + 1) * 2); }
			else if (nextInstruction == 0x40) { uint8_t amount = GetNextInstructionByte(); JumpForwardAmountInstruction(amount); }
			else if (nextInstruction == 0x41) { uint8_t amount = GetNextInstructionByte() * 2; JumpForwardAmountInstruction(amount); }
			else if (nextInstruction == 0x2D) { // Function end
				functionEnd = _currentInstruction - 1;
				break;
			}
		}
		_functions[f] = std::vector<uint8_t>(_instructionStream.begin() + functionStart, _instructionStream.begin() + functionEnd);
	}
	void TTFInstructionExecutor::EndFunctionDefinition() {
		// Used by defineFunction
	}
	void TTFInstructionExecutor::CallFunction() {
		uint32_t f = GetNextStackUInt32();

		_currentlyExecutingFunctions.push_front(ExecutingFunction());
		_currentlyExecutingFunctions.front()._function = f;
		while (_currentlyExecutingFunctions.front()._currentInstruction < _functions[f].size()) {
			ExecuteCommand(_functions[f][_currentlyExecutingFunctions.front()._currentInstruction++]);
		}
		_currentlyExecutingFunctions.pop_front();
	}
	void TTFInstructionExecutor::LoopAndCallFunction() {
		uint32_t f = GetNextStackUInt32();
		uint32_t count = GetNextStackUInt32();
		for (uint32_t i = 0; i < count; i++) {
			_currentlyExecutingFunctions.push_front(ExecutingFunction());
			_currentlyExecutingFunctions.front()._function = f;
			while (_currentlyExecutingFunctions.front()._currentInstruction < _functions[f].size()) {
				ExecuteCommand(_functions[f][_currentlyExecutingFunctions.front()._currentInstruction++]);
			}
			_currentlyExecutingFunctions.pop_front();
		}
	}
	void TTFInstructionExecutor::InstructionDefinition() {
		THROW("InstructionDefinition not implemented yet");
	}

	// Debugging
	void TTFInstructionExecutor::Debug() {
		uint32_t number = GetNextStackUInt32();
		LOG("Debug message from font -> " + std::to_string(number));
	}

	// Miscellaneous
	void TTFInstructionExecutor::GetInformation() {
		int32_t selector = GetNextStackInt32();
		int32_t result = 0;

		if (selector & 0x00000001) { // Version request
			result |= 40;
		} else if (selector & 0x00000002) { // GLYF rotated, not implemented yet
			result |= 0 & 0x00000100;
		} else if (selector & 0x00000004) { // GLYF stretched, not implemented yet
			result |= 0 & 0x00000200;
		}
		// Ignore all the other querries because we will always return 0 for the rest

		PushStack(result);
	}
	void TTFInstructionExecutor::GetVariation() {
		// No support for font variations, will never be called
	}

}
}