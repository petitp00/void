#version 330 core

// Based on: http://blog.simonrodriguez.fr/articles/30-07-2016_implementing_fxaa.html

out vec4 outColor;
in vec2 TexCoords;
uniform sampler2D screenTex;
uniform vec2 inverseScreenSize; // (1/w, 1/h)

float rgb2luma(vec3 rgb) {
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float QUALITY(int i) {
	if (i <= 4) return 1;
	if (i == 5) return 1.5;
	if (i > 6 && i < 10) return 2.0;
	if (i == 10) return 4.0;
	if (i == 11) return 8.0;
}

float EDGE_THRESHOLD_MIN = 0.0312;
float EDGE_THRESHOLD_MAX = 0.125;

int ITERATIONS = 12;

float SUBPIXEL_QUALITY = 0.75;

void main()
{
	vec3 colorCenter = texture(screenTex, TexCoords).rgb;
	float lumaCenter = rgb2luma(colorCenter);

	float lumaDown	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2( 0, -1)).rgb);
	float lumaUp	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2( 0,  1)).rgb);
	float lumaLeft	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2(-1,  0)).rgb);
	float lumaRight	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2( 1,  0)).rgb);

	// find min and max luma around fragment
	float lumaMin = min(lumaCenter, min(
					min(lumaDown, lumaUp),
					min(lumaLeft, lumaRight)));
	float lumaMax = max(lumaCenter, max(
					max(lumaDown, lumaUp),
					max(lumaLeft, lumaRight)));

	// compute the delta. (high delta means edge)
	float lumaRange = lumaMax - lumaMin;

	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax*EDGE_THRESHOLD_MAX)) {
		// edge not detected (or really dark area)
		outColor = vec4(colorCenter, 1);
		return;
	}
	/*else { // test to see what is considered a border
		outColor = vec4(1, 0, 0, 1);
		return;
	}*/

	float lumaDownLeft	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2(-1, -1)).rgb);
	float lumaUpRight	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2( 1,  1)).rgb);
	float lumaUpLeft	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2(-1,  1)).rgb);
	float lumaDownRight	= rgb2luma(textureOffset(screenTex, TexCoords, ivec2( 1, -1)).rgb);

	float lumaDownUp	= lumaDown + lumaUp;
	float lumaLeftRight	= lumaLeft + lumaRight;

	float lumaLeftCorners	= lumaDownLeft	+ lumaUpLeft;
	float lumaDownCorners	= lumaDownLeft	+ lumaDownRight;
	float lumaRightCorners	= lumaDownRight	+ lumaUpRight;
	float lumaUpCorners		= lumaUpRight	+ lumaUpLeft;

	float edgeH = abs(-2.0 * lumaLeft + lumaLeftCorners) +
				  abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 +
				  abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeV = abs(-2.0 * lumaUp + lumaUpCorners) +
				  abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 +
				  abs(-2.0 * lumaDown + lumaDownCorners);

	bool isH = (edgeH >= edgeV);

	float luma1 = isH ? lumaDown : lumaLeft;
	float luma2 = isH ? lumaUp : lumaRight;

	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	bool is1Steepest = abs(gradient1) >= abs(gradient2);

	// gradient in the corresponding direction, normalized
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

	float stepLength = isH ? inverseScreenSize.y : inverseScreenSize.x;

	float lumaLocalAverage = 0.0;
	if (is1Steepest) {
		stepLength *= -1; // switch direction
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else {
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}

	vec2 currentUv = TexCoords;
	if (isH) {
		currentUv.y += stepLength * 0.5;
	}
	else {
		currentUv.x += stepLength * 0.5;
	}

	vec2 offset = isH ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);
	vec2 uv1 = currentUv - offset;
	vec2 uv2 = currentUv + offset;

	float lumaEnd1 = rgb2luma(texture(screenTex, uv1).rgb);
	float lumaEnd2 = rgb2luma(texture(screenTex, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	if (!reached1) {
		uv1 -= offset;
	}
	if (!reached2) {
		uv2 += offset;
	}
	
	if (!reachedBoth) {
		for (int i = 2; i != ITERATIONS; ++i) {
			if (!reached1) {
				lumaEnd1 = rgb2luma(texture(screenTex, uv1).rgb);
				lumaEnd1 -= lumaLocalAverage;
			}
			if (!reached2) {
				lumaEnd2 = rgb2luma(texture(screenTex, uv2).rgb);
				lumaEnd2 -= lumaLocalAverage;
			}

			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			if (!reached1) {
				uv1 -= offset * QUALITY(i);
			}
			if (!reached2) {
				uv2 += offset * QUALITY(i);
			}

			if (reachedBoth) { break; }
		}
	}

	float dist1 = isH ? (TexCoords.x - uv1.x) : (TexCoords.y - uv1.y);
	float dist2 = isH ? (uv2.x - TexCoords.x) : (uv2.y - TexCoords.y);

	bool isDirection1 = dist1 < dist2;
	float distFinal = min(dist1, dist2);
	float edgeThickness = dist1 + dist2;
	float pixelOffset = -distFinal / edgeThickness + 0.5;

	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
	float finalOffset = correctVariation ? pixelOffset : 0.0;

	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	finalOffset = max(finalOffset, subPixelOffsetFinal);


	vec2 finalUv = TexCoords;
	if (isH) {
		finalUv.y += finalOffset * stepLength;
	}
	else {
		finalUv.x += finalOffset * stepLength;
	}

	vec3 finalColor = texture(screenTex, finalUv).rgb;
	outColor = vec4(finalColor, 1.0);
}
