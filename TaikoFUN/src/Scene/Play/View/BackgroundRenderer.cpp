#include "BackgroundRenderer.h"
#include "Scene/Play/Logic/ChartPlayer.h"
#include "Skin/SkinData.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace {
    // Provisional settings approved in the motion preview, in 1280 x 720 units.
    constexpr double posterWidth = 1280.0, posterHeight = 720.0;
    constexpr double posterPi = 3.14159265358979323846;
    constexpr double posterDelay = 3.0, posterRampEnd = 130.0;
    constexpr double posterInitialRatio = 0.05, posterSpacing = 900.0;
    constexpr double posterTurnWidth = 180.0;
    constexpr double posterLayerSpeeds[] = {80.0, 120.0, 160.0, 200.0};
    constexpr size_t posterArcSegments = 4096;
    struct PosterPoint { double x, y; };
    constexpr PosterPoint posterCenters[] = {{1350.0, 1000.0}, {1480.0, 1120.0}, {1650.0, 1380.0}};
    struct PosterObject {
        const char* textureKey;
        double centerX, centerY, scale, angle;
        double width, height;
        int group, layer;
        bool linear;
    };
    // Initial Illustrator placement; row order is retained within each layer.
    constexpr PosterObject posterObjects[] = {
		{ "play/Poster/asset_15", 1034.195, 109.651, 0.2995909, -0.0003670, 444.5929, 193.8353, 0, 1, false },
		{ "play/Poster/asset_21", 961.019, 330.086, 0.2992947, 0.0002969, 444.1533, 193.6437, 1, 1, false },
		{ "play/Poster/asset_20", 1108.183, 548.989, 0.3029486, 0.0041856, 453.8170, 200.2490, 2, 0, false },
		{ "play/Poster/asset_21", 1226.996, 689.222, 0.2992947, 0.0002969, 444.1533, 193.6437, 2, 1, false },
		{ "play/Poster/asset_14", 615.113, 20.317, 0.2999821, -0.0003110, 294.8824, 143.9914, 0, 1, false },
		{ "play/Poster/asset_11", 986.058, 366.132, 0.3013538, 0.0002829, 396.5816, 129.2808, 1, 2, false },
		{ "play/Poster/asset_01", 36.939, 224.976, 0.2997734, 0.0000913, 299.7734, 410.0900, 0, 1, false },
		{ "play/Poster/asset_09", 434.234, 352.713, 0.3007351, -0.0001660, 396.0681, 129.0154, 1, 0, false },
		{ "play/Poster/asset_03", 130.547, 453.533, 0.2995656, -0.0000136, 340.9057, 451.4454, 0, 1, false },
		{ "play/Poster/asset_04", 101.423, 697.673, 0.2992135, 0.0000295, 259.1189, 312.6781, 0, 1, false },
		{ "play/Poster/asset_05", 107.632, 755.024, 0.3007861, 0.0009299, 203.6322, 371.4708, 0, 1, false },
		{ "play/Poster/asset_02", 291.529, 167.828, 0.2980859, 0.0005070, 292.7204, 77.8004, 0, 1, false },
		{ "play/Poster/asset_27", 447.360, 541.580, 0.2990149, -0.0000342, 393.5036, 128.5764, 1, 0, false },
		{ "play/Poster/asset_24", 588.274, 658.577, 0.2994814, -0.0000756, 344.1041, 78.1646, 1, 1, false },
		{ "play/Poster/asset_07", 403.644, 704.668, 0.2995873, 0.0003515, 165.6718, 360.1039, 0, 0, false },
		{ "play/Poster/asset_08", 532.207, 439.459, 0.3003504, -0.0001426, 300.3504, 385.3496, 1, 1, false },
		{ "play/Poster/asset_26", 502.177, 782.204, 0.2988821, 0.0005257, 258.8319, 312.6307, 1, 1, false },
		{ "play/Poster/asset_22", 863.131, 677.467, 0.3004110, 0.0002281, 395.3409, 128.8763, 2, 1, false },
		{ "play/Poster/asset_25", 612.105, 776.522, 0.3101166, 0.0006123, 215.2209, 387.0255, 1, 1, false },
		{ "play/Poster/asset_12", 482.569, 253.995, 0.2824497, -0.0071309, 279.9077, 51.9707, 1, 3, false },
		{ "play/Poster/asset_13", 645.810, 196.391, 0.4194045, -0.0238265, 100.2377, 21.3896, 0, 2, false },
		{ "play/Poster/asset_17", 735.711, 397.092, 0.3037163, 0.0010179, 317.6872, 413.3579, 1, 1, false },
		{ "play/Poster/asset_10", 233.093, 377.730, 0.2950039, 0.0000291, 289.9888, 49.8557, 0, 2, false },
		{ "play/Poster/asset_23", 723.077, 561.846, 0.3290511, 0.0000000, 90.1600, 90.1600, 1, 2, true },
		{ "play/Poster/asset_23", 836.843, 462.325, 0.3117810, 0.0000000, 85.4280, 85.4280, 1, 2, true },
		{ "play/Poster/asset_23", 941.817, 370.495, 0.3003978, 0.0000000, 82.3090, 82.3090, 1, 2, true },
		{ "play/Poster/asset_23", 1036.721, 287.474, 0.2936350, 0.0000000, 80.4560, 80.4560, 1, 2, true },
		{ "play/Poster/asset_16", 1119.608, 214.966, 0.3053002, 0.0000000, 79.6834, 79.3781, 0, 2, true },
		{ "play/Poster/asset_16", 1186.978, 156.031, 0.3040449, 0.0000000, 79.3557, 79.0517, 0, 2, true },
		{ "play/Poster/asset_16", 1228.082, 120.074, 0.3038760, 0.0000000, 79.3116, 79.0078, 0, 2, true },
		{ "play/Poster/asset_19", 941.696, 369.034, 0.2831072, -0.0119363, 143.2522, 143.2522, 1, 3, true },
		{ "play/Poster/asset_06", 237.078, 739.845, 0.2982489, -0.0002463, 187.3003, 297.3542, 0, 1, false },
		{ "play/Poster/asset_16", 1281.828, 182.539, 0.3038760, 0.0000000, 79.3116, 79.0078, 0, 1, false },
		{ "play/Poster/asset_16", 1296.053, 251.391, 0.3038760, 0.0000000, 79.3116, 79.0078, 1, 1, false },
    };
    struct PosterOrbit {
        double centerX = 0.0, centerY = 0.0;
        double radiusX = 0.0, radiusY = 0.0, initialAngle = 0.0;
        double minimum = 0.0, length = 0.0;
        std::vector<double> arc;
    };
    struct PosterInstance { size_t sourceIndex; double offset; };
    struct PosterMotion {
        std::vector<PosterOrbit> orbits;
        std::vector<PosterInstance> instances;
        PosterPoint lineDirection;
    };
    double PosterWrap(double value, double length) {
        return std::fmod(std::fmod(value, length) + length, length);
    }
    PosterPoint PosterGuide(double t) {
        const double u = 1.0 - t;
        return {3*u*u*t*522.06 + 3*u*t*t*1043.28 + t*t*t*1280.0,
                3*u*t*t*233.93 + t*t*t*720.0};
    }
    double PosterGuideY(double x) {
        double lo = 0.0, hi = 1.0;
        for (int i = 0; i < 15; ++i) {
            const double t = (lo + hi) / 2.0;
            if (PosterGuide(t).x < x) lo = t; else hi = t;
        }
        return PosterGuide((lo + hi) / 2.0).y;
    }
    double PosterOrbitAngle(const PosterOrbit& orbit, double distance) {
        const double d = PosterWrap(distance, orbit.length);
        const auto upper = std::upper_bound(orbit.arc.begin(), orbit.arc.end(), d);
        const size_t lo = (std::min)(static_cast<size_t>(upper - orbit.arc.begin() - 1), posterArcSegments - 1);
        const double fraction = (d - orbit.arc[lo]) / (orbit.arc[lo + 1] - orbit.arc[lo]);
        return orbit.initialAngle + (lo + fraction) * (2.0 * posterPi / posterArcSegments);
    }
    PosterPoint PosterPosition(const PosterObject& object, const PosterOrbit& orbit,
                               const PosterPoint& direction, double distance) {
        if (object.linear) {
            const double shift = PosterWrap(distance - orbit.minimum, orbit.length) + orbit.minimum;
            return {object.centerX + direction.x * shift, object.centerY + direction.y * shift};
        }
        const double angle = PosterOrbitAngle(orbit, distance);
        return {orbit.centerX + orbit.radiusX * std::cos(angle),
                orbit.centerY + orbit.radiusY * std::sin(angle)};
    }
    double PosterDrawAngle(const PosterObject& object, const PosterOrbit& orbit, const PosterPoint& point) {
        if (object.linear) return object.angle;
        const double initial = std::atan2(orbit.centerY - object.centerY, orbit.centerX - object.centerX);
        const double current = std::atan2(orbit.centerY - point.y, orbit.centerX - point.x);
        return object.angle + std::atan2(std::sin(current - initial), std::cos(current - initial));
    }
    bool PosterIntersects(const PosterObject& object, const PosterPoint& point, double angle, double margin) {
        const double c = std::abs(std::cos(angle)), s = std::abs(std::sin(angle));
        const double x = (c * object.width + s * object.height) / 2.0;
        const double y = (s * object.width + c * object.height) / 2.0;
        return point.x + x + margin > 0.0 && point.x - x - margin < posterWidth &&
               point.y + y + margin > 0.0 && point.y - y - margin < posterHeight;
    }
    PosterMotion BuildPosterMotion() {
        PosterMotion motion;
        const double dx = posterObjects[29].centerX - posterObjects[23].centerX;
        const double dy = posterObjects[29].centerY - posterObjects[23].centerY;
        const double magnitude = std::hypot(dx, dy);
        motion.lineDirection = {dx / magnitude, dy / magnitude};
        const double halfSpan = std::ceil((std::hypot(posterWidth, posterHeight) + posterSpacing) / posterSpacing) * posterSpacing;
        motion.orbits.reserve(std::size(posterObjects));
        for (const auto& object : posterObjects) {
            PosterOrbit orbit;
            if (object.linear) {
                orbit.minimum = -halfSpan;
                orbit.length = halfSpan * 2.0;
            } else {
                const auto& center = posterCenters[object.group];
                orbit.centerX = center.x; orbit.centerY = center.y;
                double below = std::clamp(0.5 + (object.centerY - PosterGuideY(std::clamp(object.centerX, 0.0, posterWidth))) / posterTurnWidth, 0.0, 1.0);
                below = below * below * (3.0 - 2.0 * below);
                const double tangent = (5.0 + 40.0 * below) * posterPi / 180.0;
                const double ratio = std::clamp(std::sqrt((std::max)(1.0, center.x - object.centerX) /
                    ((std::max)(1.0, center.y - object.centerY) * std::tan(tangent))), 0.55, 4.5);
                orbit.radiusX = std::hypot(object.centerX - center.x, ratio * (object.centerY - center.y));
                orbit.radiusY = orbit.radiusX / ratio;
                orbit.initialAngle = std::atan2((object.centerY - center.y) / orbit.radiusY, (object.centerX - center.x) / orbit.radiusX);
                orbit.arc.resize(posterArcSegments + 1, 0.0);
                PosterPoint previous = {orbit.radiusX * std::cos(orbit.initialAngle), orbit.radiusY * std::sin(orbit.initialAngle)};
                for (size_t k = 1; k <= posterArcSegments; ++k) {
                    const double angle = orbit.initialAngle + k * (2.0 * posterPi / posterArcSegments);
                    const PosterPoint next = {orbit.radiusX * std::cos(angle), orbit.radiusY * std::sin(angle)};
                    orbit.arc[k] = orbit.arc[k - 1] + std::hypot(next.x - previous.x, next.y - previous.y);
                    previous = next;
                }
                orbit.length = orbit.arc.back();
            }
            motion.orbits.push_back(std::move(orbit));
        }
        for (size_t i = 0; i < std::size(posterObjects); ++i) {
            const auto& object = posterObjects[i]; const auto& orbit = motion.orbits[i];
            const size_t count = std::max<size_t>(2, static_cast<size_t>(std::ceil(orbit.length / posterSpacing)));
            const double gap = orbit.length / count;
            motion.instances.push_back({i, 0.0});
            for (size_t k = 1; k < count; ++k) {
                const double offset = k * gap;
                bool visible = false;
                if (object.linear) {
                    for (const auto& other : posterObjects) if (other.linear) {
                        const auto point = PosterPosition(other, orbit, motion.lineDirection, offset);
                        if (PosterIntersects(other, point, other.angle, 20.0)) { visible = true; break; }
                    }
                } else {
                    const auto point = PosterPosition(object, orbit, motion.lineDirection, offset);
                    visible = PosterIntersects(object, point, PosterDrawAngle(object, orbit, point), 20.0);
                }
                if (!visible) motion.instances.push_back({i, offset});
            }
        }
        std::stable_sort(motion.instances.begin(), motion.instances.end(), [](const auto& a, const auto& b) {
            const int layerA = posterObjects[a.sourceIndex].layer, layerB = posterObjects[b.sourceIndex].layer;
            return layerA != layerB ? layerA < layerB : a.sourceIndex < b.sourceIndex;
        });
        return motion;
    }
    const PosterMotion& GetPosterMotion() {
        static const PosterMotion motion = BuildPosterMotion();
        return motion;
    }
    double PosterDistance(int layer, double elapsedSeconds) {
        const double duration = posterRampEnd - posterDelay;
        const double tau = std::clamp(elapsedSeconds - posterDelay, 0.0, duration);
        return posterLayerSpeeds[layer] * (posterInitialRatio * tau + (1.0 - posterInitialRatio) * tau * tau * tau /
            (3.0 * duration * duration) + (std::max)(0.0, elapsedSeconds - posterRampEnd));
    }
}

void BackgroundRenderer::UpdatePosterBackground(double elapsedSeconds) {
    const auto& motion = GetPosterMotion();
    posterDrawObjects.clear();
    posterDrawObjects.reserve(motion.instances.size());
    for (const auto& instance : motion.instances) {
        const auto& object = posterObjects[instance.sourceIndex];
        const auto& orbit = motion.orbits[instance.sourceIndex];
        const auto point = PosterPosition(object, orbit, motion.lineDirection,
            PosterDistance(object.layer, elapsedSeconds) + instance.offset);
        posterDrawObjects.push_back({instance.sourceIndex, point.x, point.y, PosterDrawAngle(object, orbit, point)});
    }
}

BackgroundRenderer::BackgroundRenderer() {
	Init();
}

void BackgroundRenderer::Init() {
	UpdatePosterBackground(0.0);
}

void BackgroundRenderer::Update(const ChartPlayer& player) {
	UpdatePosterBackground(player.getPlaybackElapsedSec());
}

void BackgroundRenderer::Draw(const ChartPlayer& player) const {
	if (player.isGogoTime()) {
		DrawGraph(0, 0, Skin::GetTexture("play/bg_clear").handle, TRUE);
		return;
	}

	int width = 0, height = 0;
	GetDrawScreenSize(&width, &height);
	const double scale = height / posterHeight;
	DrawBox(0, 0, width, height, GetColor(64, 84, 163), TRUE);
	for (const auto& state : posterDrawObjects) {
		const auto& object = posterObjects[state.sourceIndex];
		const auto& texture = Skin::GetTexture(object.textureKey);
		DrawRotaGraphF(
			static_cast<float>(state.centerX / posterWidth * width),
			static_cast<float>(state.centerY / posterHeight * height),
			object.scale * scale, state.angle, texture.handle, TRUE);
	}
}
