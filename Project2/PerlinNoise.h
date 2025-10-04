#pragma once

#include <DirectXMath.h>
#include <cmath>
#include <array>
#include <numeric>

using namespace DirectX;

// --- 1. Static Data and Helper Struct ---
struct PerlinNoise
{
    // The permutation array P (initialized as a random arrangement of 0-255, then doubled)
    // NOTE: This must be generated/initialized properly once.
    static constexpr int P_SIZE = 512;
    alignas(16) std::array<int, P_SIZE> P;

    // The 12 predefined gradient vectors (can be extended to 16 for bitwise simplicity)
    // Represented as XMVECTORs for SIMD efficiency.
    // Example: (1, 1, 0), (-1, 1, 0), etc.
    static inline const std::array<XMVECTOR, 16> Gradients = {
        XMVectorSet(1.0f,  1.0f,  0.0f, 0.0f), XMVectorSet(-1.0f,  1.0f,  0.0f, 0.0f),
        XMVectorSet(1.0f, -1.0f,  0.0f, 0.0f), XMVectorSet(-1.0f, -1.0f,  0.0f, 0.0f),
        // ... (12 standard gradient vectors defining the 12 edges from a center)
        // ... (remaining vectors for 16-entry lookup)
    };

    // --- Constructor to initialize P (simplified for example) ---
    PerlinNoise() {
        // Initialize P with a random permutation of 0-255
        // (Real implementation requires a proper shuffle for randomness)
        std::iota(P.begin(), P.begin() + 256, 0);
        std::copy(P.begin(), P.begin() + 256, P.begin() + 256); // Copy to 512 entries
    }
};

// --- 2. Fade Function (Quintic Curve) ---
// This function performs the 6t^5 - 15t^4 + 10t^3 smoothing.
// Since it operates on a scalar, a direct function is used.
float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// --- 3. Hash and Gradient Dot Product ---
float grad(int hash, float x_dist, float y_dist, float z_dist) {
    // Selects one of the 16 gradients based on the hash (h & 0xF)
    XMVECTOR g = PerlinNoise::Gradients[hash & 0xF];
    XMVECTOR d = XMVectorSet(x_dist, y_dist, z_dist, 0.0f);

    // DirectXMath for efficient dot product
    XMVECTOR dot_product = XMVector3Dot(d, g);

    float result;
    XMStoreFloat(&result, dot_product);
    return result;
}

// --- 4. Main Noise Function ---
float noise(float x, float y, float z, const PerlinNoise& pn)
{
    // 1. Grid and Relative Coordinates
    int xi = (int)std::floor(x);
    int yi = (int)std::floor(y);
    int zi = (int)std::floor(z);

    float xf = x - xi;
    float yf = y - yi;
    float zf = z - zi;

    // 2. Faded Coordinates
    float u = fade(xf);
    float v = fade(yf);
    float w = fade(zf);

    // Pointers for fast hash lookup (using the 512 array)
    int A = pn.P[xi] + yi;
    int B = pn.P[xi + 1] + yi;
    int AA = pn.P[A] + zi;
    int AB = pn.P[A + 1] + zi;
    int BA = pn.P[B] + zi;
    int BB = pn.P[B + 1] + zi;

    // 3. Dot Products (8 corners)
    float g000 = grad(pn.P[AA], xf, yf, zf);             // Corner (0, 0, 0)
    float g100 = grad(pn.P[BA], xf - 1, yf, zf);         // Corner (1, 0, 0)
    float g010 = grad(pn.P[AB], xf, yf - 1, zf);         // Corner (0, 1, 0)
    float g110 = grad(pn.P[BB], xf - 1, yf - 1, zf);     // Corner (1, 1, 0)

    float g001 = grad(pn.P[AA + 1], xf, yf, zf - 1);     // Corner (0, 0, 1)
    float g101 = grad(pn.P[BA + 1], xf - 1, yf, zf - 1); // Corner (1, 0, 1)
    float g011 = grad(pn.P[AB + 1], xf, yf - 1, zf - 1); // Corner (0, 1, 1)
    float g111 = grad(pn.P[BB + 1], xf - 1, yf - 1, zf - 1); // Corner (1, 1, 1)

    // 4. Interpolation (Lerp)
    // Lerp across X
    float x1 = XMVectorGetX(XMVectorLerpV(XMVectorSet(g000, 0, 0, 0), XMVectorSet(g100, 0, 0, 0), XMVectorReplicate(u)));
    float x2 = XMVectorGetX(XMVectorLerpV(XMVectorSet(g010, 0, 0, 0), XMVectorSet(g110, 0, 0, 0), XMVectorReplicate(u)));
    float x3 = XMVectorGetX(XMVectorLerpV(XMVectorSet(g001, 0, 0, 0), XMVectorSet(g101, 0, 0, 0), XMVectorReplicate(u)));
    float x4 = XMVectorGetX(XMVectorLerpV(XMVectorSet(g011, 0, 0, 0), XMVectorSet(g111, 0, 0, 0), XMVectorReplicate(u)));

    // Lerp across Y
    float y1 = XMVectorGetX(XMVectorLerpV(XMVectorSet(x1, 0, 0, 0), XMVectorSet(x2, 0, 0, 0), XMVectorReplicate(v)));
    float y2 = XMVectorGetX(XMVectorLerpV(XMVectorSet(x3, 0, 0, 0), XMVectorSet(x4, 0, 0, 0), XMVectorReplicate(v)));

    // Final Lerp across Z (The final noise value)
    return XMVectorGetX(XMVectorLerpV(XMVectorSet(y1, 0, 0, 0), XMVectorSet(y2, 0, 0, 0), XMVectorReplicate(w)));
}
