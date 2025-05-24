#ifndef ENGINE_RENDERER_TTF_INSTRUCTION_EXECUTOR_H
#define ENGINE_RENDERER_TTF_INSTRUCTION_EXECUTOR_H

// See https://learn.microsoft.com/en-us/typography/opentype/spec/ for all the information about .otf files
// and https://developer.apple.com/fonts/TrueType-Reference-Manual/ for all the information about .ttf files

// The FreeType project was used to find undocumented parts of the spec

#include "core/PCH.h"
#include "util/Vec2D.h"

namespace Engine {
namespace Renderer {

	typedef int32_t F26Dot6;

#define GLYF_ON_CURVE_POINT 0x01
//#define GLYF_NOT_USED 0x02
//#define GLYF_NOT_USED 0x04
//#define GLYF_NOT_USED 0x08
//#define GLYF_NOT_USED 0x10
//#define GLYF_NOT_USED 0x20
#define GLYF_TOUCHED_X 0x40
#define GLYF_TOUCHED_Y 0x80

	class TTFInstructionExecutor {
	public:

		// Settings (!These are manditory to set!)
		void SetMaxStorageAreaSize(const size_t size);
		void SetMaxFunctions(const size_t size);
		void SetMaxTwilightPoints(const size_t size);
		void SetScale(const uint32_t unitsPerEM, const uint32_t pointSize);
		// The flags should have already been expanded so every repeat is in the array
		// The inputted points are still in FDU
		void SetOriginalGlyphInfo(const std::vector<uint8_t> flags, const std::vector<Util::Vec2F> points, const std::vector<uint16_t> endpoints);
		void AddPhantomPoints(const Util::Vec2F min, const Util::Vec2F max, const int32_t leftSideBearing, const int32_t advance, const int32_t topOrigin, const int32_t advanceHeight);

		// Getters
		inline std::vector<uint8_t>* GetNewFlags() { return &_flags; }
		std::unique_ptr<std::vector<Util::Vec2F>> GetNewPoints();
		inline float GetLeftSideBearing() {
			return FromF26Dot6<float>(_points[_points.size() - 4].x);
		}
		inline float GetAdvanceWidth() {
			return FromF26Dot6<float>(_points[_points.size() - 3].x);
		}
		inline float GetTopOrigin() {
			return FromF26Dot6<float>(_points[_points.size() - 2].y);
		}
		inline float GetAdvanceHeight() {
			return FromF26Dot6<float>(_points[_points.size() - 1].y);
		}

		// Instruction Stream
		inline void AddToInstructionStream(const std::vector<uint8_t>* instructions) {
			_instructionStream.insert(_instructionStream.end(), instructions->begin(), instructions->end());
		}
		inline void AddToInstructionStream(const uint8_t value) {
			_instructionStream.push_back(value);
		}
		inline void SetInstructionStream(const std::vector<uint8_t>* instructions) {
			_instructionStream = *instructions;
		}
		inline void ReserveInstructionStreamSpace(const size_t size) {
			_instructionStream.reserve(size);
		}

		// Control Values
		inline void SetControlValues(const std::vector<int16_t>* values) {
			_controlValues.reserve(values->size());
			for (int16_t value : *values) {
				_controlValues.push_back(ToF26Dot6(value));
			}
		}
		inline void AddControlValue(const int16_t value) {
			_controlValues.push_back(ToF26Dot6(value));
		}
		inline void ReserveControlValueSpace(const size_t size) {
			_controlValues.reserve(size);
		}

		void ExecuteStack();

		// Used to store the state after the FPGM and CVT program
		void StoreGraphicsState();
		void BindStoredGraphicsState();

		std::set<uint8_t> _usedInstructions;

	private:

		inline bool IsNextInstructionLeft();
		inline uint8_t GetNextInstructionByte();
		inline int32_t GetNextInstructionPaddedWord();
		inline void JumpForwardAmountInstruction(const int32_t amount);

		inline void PushStack(const bool value);
		inline void PushStack(const uint8_t value);
		inline void PushStack(const int32_t value);
		inline void PushStack(const uint32_t value);
		inline int32_t GetNextStackInt32();
		inline uint32_t GetNextStackUInt32();
		inline F26Dot6 GetNextStackF26Dot6();

		void ExecuteCommand(const uint8_t command);

		// F26Dot6 helpers
		template<class T>
		inline F26Dot6 ToF26Dot6(const T value) {
			return (F26Dot6)(value * 0x40);
		}
		template<class T>
		inline Util::Vec2<F26Dot6> ToF26Dot6(const Util::Vec2<T> value) {
			return Util::Vec2<F26Dot6>(ToF26Dot6(value.x), ToF26Dot6(value.y));
		}
		template<class T>
		inline T FromF26Dot6(const F26Dot6 value) {
			return value / ((T)0x40);
		}
		template<class T>
		inline Util::Vec2<T> FromF26Dot6(const Util::Vec2<F26Dot6> value) {
			return Util::Vec2<T>(value.x / ((T)0x40), value.y / ((T)0x40));
		}

		// Point getters and setters
		inline Util::Vec2<F26Dot6> GetPoint(const uint32_t p, const uint32_t zone);
		inline Util::Vec2<F26Dot6> GetOriginalPoint(const uint32_t p, const uint32_t zone);
		inline void SetPoint(const uint32_t p, const uint32_t zone, Util::Vec2<F26Dot6> value);
		inline void AddToPoint(const uint32_t p, const uint32_t zone, Util::Vec2<F26Dot6> additive);
		inline void AddToPointWithoutTouching(const uint32_t p, const uint32_t zone, Util::Vec2<F26Dot6> additive);
		inline void SetPointTouched(const uint32_t p, const uint32_t zone);
		inline void SetPointTouched(const uint32_t p, const uint32_t zone, const bool x, const bool y);
		inline void SetPointUntouched(const uint32_t p, const uint32_t zone);
		inline void SetPointUntouched(const uint32_t p, const uint32_t zone, const bool x, const bool y);
		inline void MovePointToProjectedValue(const uint32_t p, const uint32_t zone, const F26Dot6 value);
		inline F26Dot6 ProjectPoint(const Util::Vec2<F26Dot6> point);
		inline F26Dot6 DualProjectPoint(const Util::Vec2<F26Dot6> point);
		// CVT
		inline F26Dot6 GetCVTValue(const uint32_t n);
		inline void SetCVTValue(const uint32_t n, const F26Dot6 value);
		// Utils
		inline Util::Vec2<F26Dot6> RoundToGrid(const Util::Vec2<F26Dot6> p);

		size_t _currentInstruction = 0;
		std::vector<uint8_t> _instructionStream;
		std::deque<int32_t> _interpreterStack;
		std::vector<F26Dot6> _controlValues;
		bool _controlValuesScaled = false;
		std::vector<uint32_t> _storageArea;

		struct ExecutingFunction {
			uint32_t _function;
			size_t _currentInstruction;
		};
		std::deque<ExecutingFunction> _currentlyExecutingFunctions;
		std::vector<std::vector<uint8_t>> _functions;

		std::vector<uint8_t> _originalFlags;
		std::vector<Util::Vec2<F26Dot6>> _originalPoints;
		std::vector<uint8_t> _flags;
		std::vector<Util::Vec2<F26Dot6>> _points;
		std::vector<uint16_t> _endpoints;

		// Graphics State
		struct GraphicsState {
			double _pointScale = 0;
			uint32_t _pointSize = 0;
			uint32_t _pixelsPerEM = 0;

			std::vector<Util::Vec2<F26Dot6>> _twilightPoints;
			std::vector<Util::Vec2<F26Dot6>> _originalTwilightPoints;
			std::vector<Util::Vec2<F26Dot6>> _IUPDeltas;

			Util::Vec2F _projectionVector = Util::Vec2F(1, 0);
			Util::Vec2F _freedomVector = Util::Vec2F(1, 0);
			Util::Vec2F _dualProjectionVector;

			uint32_t _refrencePoint0 = 0;
			uint32_t _refrencePoint1 = 0;
			uint32_t _refrencePoint2 = 0;

			uint32_t _zonePointer0 = 1;
			uint32_t _zonePointer1 = 1;
			uint32_t _zonePointer2 = 1;

			enum class RoundState {
				ToHalfGrid,
				ToGrid,
				ToDoubleGrid,
				DownToGrid,
				UpToGrid,
				None,
				Super,
				Super45Degree
			} _roundingState = RoundState::ToGrid;
			float _superRoundPeriod = 0;
			float _superRoundPhase = 0;
			float _superRoundThreshold = 0;

			uint32_t _loopVariable = 1;
			F26Dot6 _minimumDistance = 64;
			// Execution control
			// Execution conversion
			// Scan conversion

			F26Dot6 _CVTCutIn = 0x44; // 17/16
			F26Dot6 _singleWidthCutIn = 0;
			F26Dot6 _singleWidth = 0;
			bool _autoFlip = true;
			uint32_t _deltaBase = 9;
			uint32_t _deltaShift = 3;
		};
		GraphicsState _state;
		GraphicsState _storedState;


		// Helpers
		void SetSuperRound(const uint32_t n, const float gridPeriod);
		// Phase should always be positive
		F26Dot6 Round(const F26Dot6 n, const F26Dot6 period, const F26Dot6 phase, const F26Dot6 threshold);
		F26Dot6 Round(const F26Dot6 n, const float period, const float phase, const float threshold);
		F26Dot6 Round(const F26Dot6 n);

		// ===================================================
		// Instructions
		// ===================================================
		
		// Push commands
		void PushNBytes();
		void PushNWords();
		void PushBytes(const uint8_t num);
		void PushWords(const uint8_t num);

		// Store commands
		void ReadStore();
		void WriteStore();

		// CVT commands
		void WriteCVTInPixels();
		void WriteCVTInFDU();
		void ReadCVT();

		// ===================================================
		// Graphics state

		// Vectors commands
		void SetFreedomAndProjectionVectorToAxis(const bool xOrY);
		void SetProjectionVectorToAxis(const bool xOrY);
		void SetFreedomVectorToAxis(const bool xOrY);
		void SetProjectionVectorToLine(const bool perpendicular);
		void SetFreedomVectorToLine(const bool perpendicular);
		void SetFreedomVectorToProjectionVector();
		void SetDualProjectionVectorToLine(const bool perpendicular);
		void SetProjectionVectorFromStack();
		void SetFreedomVectorFromStack();
		void GetProjectionVector();
		void GetFreedomVector();

		// Reference points
		void SetReferencePoint0();
		void SetReferencePoint1();
		void SetReferencePoint2();

		// Zone pointers
		void SetZonePointer0();
		void SetZonePointer1();
		void SetZonePointer2();
		void SetZonePointerS();

		// Rounding commands
		void RoundToHalfGrid();
		void RoundToGrid();
		void RoundToDoubleGrid();
		void RoundDownToGrid();
		void RoundUpToGrid();
		void RoundNone();
		void RoundSuper();
		void RoundSuper45Degree();

		// Miscellaneous graphic state commands
		void SetLoopVariable();
		void SetMinimumDistance();
		void SetInstructionExecutionControl();
		void SetScanConversionControl();
		void SetScanType();
		void SetCVTCutIn();
		void SetSingleWidthCutIn();
		void SetSingleWidth();
		void SetAutoFlipOn();
		void SetAutoFlipOff();
		void SetDeltaBase();
		void SetDeltaShift();

		//
		// ===================================================

		// Reading and writing data
		void GetCoordinateProjectedOnProjectionVector(const bool originalOutline);
		void SetCoordinateFromStackUsingGraphicsVectors();
		void MeasureDistance(const bool originalOutline);
		void MeasurePixelsPerEM();
		void MeasurePointSize();

		// Outline managing
		void FlipPoint();
		void FlipRangeOn();
		void FlipRangeOff();
		void ShiftPointByLastPoint(const bool a);
		void ShiftContourByLastPoint(const bool a);
		void ShiftZoneByLastPoint(const bool a);
		void ShiftPointByPixelAmount();
		void MoveStackIndirectRelativePoint(const bool setRP0);
		void MoveDirectAbsolutePoint(const bool applyRounding);
		void MoveIndirectAbsolutePoint(const bool applyRounding);
		void MoveDirectRelativePoint(const bool setRP0, const bool minimumDistance, const bool applyRounding, const uint8_t distanceType);
		void MoveIndirectRelativePoint(const bool setRP0, const bool minimumDistance, const bool applyRounding, const uint8_t distanceType);
		void AlignRelativePoint();
		void MovePointPToIntersection();
		void AlignPoints();
		void InterpolatePointByLastRelativeStretch();
		void UntouchPoint();
		void InterpolateUntouchedPointsThroughTheOutline(const bool xOrY);

		// Exceptions
		void DeltaExceptionP1();
		void DeltaExceptionP2();
		void DeltaExceptionP3();
		void DeltaExceptionC1();
		void DeltaExceptionC2();
		void DeltaExceptionC3();

		// Managing the stack
		void DuplicateTop();
		void PopTop();
		void ClearStack();
		void SwapTop();
		void ReturnDepth();
		void CopyIndexedToTop();
		void MoveIndexedToTop();
		void RollTopThree();

		// Flow control
		void IfTest();
		void Else();
		void EndIf();
		void JumpRelativeOnTrue();
		void Jump();
		void JumpRelativeOnFalse();

		// Logical functions
		void LessThan();
		void LessThanOrEqual();
		void GreaterThan();
		void GreaterThanOrEqual();
		void Equal();
		void NotEqual();
		void Odd();
		void Even();
		void LogicalAnd();
		void LogicalOr();
		void LogicalNot();

		// Math functions
		void Add();
		void Subtract();
		void Divide();
		void Multiply();
		void AbsoluteValue();
		void Negate();
		void Floor();
		void Ceiling();
		void MaximumTop2();
		void MinimumTop2();

		// Round
		void RoundInstruction(const uint8_t ab);
		void NoRound(const uint8_t ab);

		// Defining functions
		void DefineFunction();
		void EndFunctionDefinition();
		void CallFunction();
		void LoopAndCallFunction();
		void InstructionDefinition();

		// Debugging
		void Debug();

		// Miscellaneous
		void GetInformation();
		void GetVariation();

	};

}
}

#endif