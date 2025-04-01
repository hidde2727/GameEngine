#ifndef ENGINE_UTILS_EQUATIONSOLVERS_H
#define ENGINE_UTILS_EQUATIONSOLVERS_H

#include "core/PCH.h"

namespace Engine {
namespace Utils {

	template <typename T>
	concept FloatingPoint = std::floating_point<T>;

#define PI 3.141592653589793
#define ROOT_MAX_SEARCH_ITERATIONS 1024
#define BRACKET_SEARCH_MAX_STEPS 32
#define PRECISION 0.000000001

	template<FloatingPoint T>
	// Solves ax^3 + bx^2 + cx + d = 0
	// Returns the solutions in the T x[3] and returns the amount of solutions
	int SolveCubic(T x[3], T a, T b, T c, T d) {
		T q = ((2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a));
		T p = ((3 * a * c - b * b) / (3 * a * a));

		T discriminant = 4 * p * p * p + 27 * q * q;
		if (discriminant > 0) {
			// There is 1 real solution
			// Use the 'General cubic formula'
			// https://en.wikipedia.org/wiki/Cubic_equation#:~:text=2%20or%203.-,General%20cubic%20formula,-%5Bedit%5D
			T D0 = b * b - 3 * a * c;
			T D1 = 2 * b * b * b - 9 * a * b * c + 27 * a * a * d;
			T C = cbrt(((D1 + sqrt((D1 * D1) - (4 * D0 * D0 * D0))) / (2)));

			x[0] = -((1) / (3 * a)) * (b + C + ((D0) / (C)));

			return 1;
		}
		else {
			// There are 3 solutions
			// Use the 'Trigonometric solution for three real roots'
			// https://en.wikipedia.org/wiki/Cubic_equation#:~:text=Trigonometric%20solution%20for%20three%20real%20roots
			T middle = (T)((1. / 3.) * acos(((3. * q) / (2. * p)) * sqrt(-((3.) / (p)))));

			x[0] = 2 * sqrt(-((p) / (3.))) * cos(middle) - (T)((b) / (3 * a));
			x[1] = 2 * sqrt(-((p) / (3.))) * cos(middle - 2 * ((PI) / (3.))) - (T)((b) / (3 * a));
			x[2] = 2 * sqrt(-((p) / (3.))) * cos(middle - 4 * ((PI) / (3.))) - (T)((b) / (3 * a));

			return 3;
		}
	}

	template<FloatingPoint T>
	inline T quartic(T x, T a, T b, T c, T d, T e) {
		//return (a * x * x * x * x + b * x * x * x + c * x * x + d * x + e);
		return (((a*x+b)*x+c)*x+d)*x+e;
	}

	template<FloatingPoint T>
	// Solves ax^4 + bx^3 + cx^2 + dx + e = 0
	// Returns the solutions in the T x[4] and returns the amount of solutions
	int SolveQuartic(T x[4], T a, T b, T c, T d, T e) {
		T p = ((8 * a * c - 3 * b * b) / (8 * a * a));
		T D = 64 * a * a * a * e - 16 * a * a * c * c + 16 * a * b * b * c - 16 * a * a * b * d - 3 * b * b * b * b;
		T D0 = c * c - 3 * b * d + 12 * a * e;
		T D1 = 2 * c * c * c - 9 * b * c * d + 27 * b * b * e + 27 * a * d * d - 72 * a * c * e;

		if (D1 * D1 - 4 * D0 * D0 * D0 > 0) {
			// The equation has 2 roots, solvable with the 'general quartic function'
			// https://en.wikipedia.org/wiki/Quartic_function#:~:text=is%20not%20possible.-,General%20formula%20for%20roots,-%5Bedit%5D
			T q = ((b * b * b - 4 * a * b * c + 8 * d * a * a) / (8 * a * a * a));

			T Q = (T)cbrt(((D1 + sqrt(D1 * D1 - 4 * D0 * D0 * D0)) / 2.));
			T S = (T)0.5 * sqrt(-(2. / 3.) * p + (1. / (3 * a)) * (Q + (D0 / Q)));

			int currentValidRoot = 0;
			T x1 = -((b) / (4 * a)) - S + 0.5 * sqrt(-4 * S * S - 2 * p + (q / S));
			if (!std::isnan(x1)) { x[currentValidRoot++] = x1; }
			T x2 = -((b) / (4 * a)) - S - 0.5 * sqrt(-4 * S * S - 2 * p + (q / S));
			if (!std::isnan(x2)) { x[currentValidRoot++] = x2; }
			T x3 = -((b) / (4 * a)) + S + 0.5 * sqrt(-4 * S * S - 2 * p - (q / S));
			if (!std::isnan(x3)) { x[currentValidRoot++] = x3; }
			T x4 = -((b) / (4 * a)) + S - 0.5 * sqrt(-4 * S * S - 2 * p - (q / S));
			if (!std::isnan(x4)) { x[currentValidRoot++] = x4; }

			return currentValidRoot;
		}
		else if (p < 0 && D < 0) {
			// There are 4 other roots
			// To solve this with the 'general quartic function' you need complex numbers so we solve it with another 'Newton fourier method'
			// And then solve it as a cubic function
			T x1 = 1;

			T xn_2 = -1;
			T fx = 0;
			for (int i = 0; i < ROOT_MAX_SEARCH_ITERATIONS; i++) {
				// Use the 'Secant method'
				// https://en.wikipedia.org/wiki/Secant_method
				fx = quartic(x1, a, b, c, d, e);
				if (fx <= PRECISION && fx >= -PRECISION)
					break;
				T fxn_2 = quartic(xn_2, a, b, c, d, e);
				T oldX = x1;
				x1 = x1 - fx * ((x1 - xn_2) / (fx - fxn_2));
				xn_2 = oldX;
			}
			// Check if the algortihm hasn't failed
			if(fx >= PRECISION || fx <= -PRECISION) return 0;
			x[0] = x1;

			// Convert the quartic function to a cubic function by dividing by (x + solutionX)
			T a2 = a;
			T b2 = b + a2 * x1;
			T c2 = c + b2 * x1;
			T d2 = d + c2 * x1;

			// This can be uncommented if you want to make sure no false roots slip by
			// Shouldn't be needed because the secant method already checks
			//T remainder = e + d2 * x1;
			//if (remainder > 1 || remainder < -1)
			//	return 1;

			int numSolutions = SolveCubic(&x[1], a2, b2, c2, d2);

			return 1 + numSolutions;
		}
		else {
			// There aren't any real roots remaining
			return 0;
		}
	}

	template<FloatingPoint T>
	inline T quintic(T x, T a, T b, T c, T d, T e, T f) {
		//return (a * x * x * x * x * x + b * x * x * x * x + c * x * x * x + d * x * x + e * x + f);
		return ((((a*x+b)*x+c)*x+d)*x+e)*x+f;
	}

	template<FloatingPoint T>
	// Solves ax^5 + bx^4 + cx^3 + dx^2 + ex + f = 0
	// Returns the solutions in the T x[5] and returns the amount of solutions
	int SolveQuintic(T x[5], T a, T b, T c, T d, T e, T f) {
		// Start by searching for a root
		T x1 = 100000000.;

		T fx = 0;
		{
			T xA = -1;
			T xB = 1;
			int i = 0;
			// Start by finding a bracket (two places of opposite sign in the function)
			while (true) {
				T fxA = quintic(xA, a, b, c, d, e, f);
				T fxB = quintic(xB, a, b, c, d, e, f);
				if ((fxA < 0 && fxB > 0) || (fxB < 0 && fxA > 0)) break; // Found a bracket
				xA *= 2;
				xB *= 2;
				ASSERT(i++ >= BRACKET_SEARCH_MAX_STEPS, "Failed to find a quintic bracket for the bisection method")
			}
			// Use the 'Bisection Method' to find a root
			// https://en.wikipedia.org/wiki/Bisection_method
			for (int i = 0; i < ROOT_MAX_SEARCH_ITERATIONS; i++) {
				T xC = (xA + xB) / 2;
				T fxC = quintic(xC, a, b, c, d, e, f);
				if ((fxC <= PRECISION && fxC >= -PRECISION) || ((xB - xA) / 2 < PRECISION)) {
					x1 = xC;
					break;
				}
				if (std::signbit(fxC) == std::signbit(quintic(xA, a, b, c, d, e, f)))
					xA = xC;
				else
					xB = xC;
			}

			// Check if the bisection method found a root
			ASSERT(x1 == 100000000., "Failed to find a quintic solution")

			x[0] = x1;
		}

		// Convert the quintic function to a quartic function by dividing by (x + solutionX)
		T a2 = a;
		T b2 = b + a2 * x1;
		T c2 = c + b2 * x1;
		T d2 = d + c2 * x1;
		T e2 = e + d2 * x1;

		// This can be uncommented if you want to make sure no false roots slip by
		// Shouldn't be needed because the bisection method already checks
		//T remainder = f + e2 * x1;
		//if (remainder > 1 || remainder < -1)
 		//	return 1;

		int numSolutions = SolveQuartic(&x[1], a2, b2, c2, d2, e2);

		return 1 + numSolutions;
	}

}
}

#endif