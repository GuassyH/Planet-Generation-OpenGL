#version 460 core


in vec2 fragCoord;
uniform vec3 atmosphereCentre;
uniform vec3 camPos;
uniform vec3 sunPos;

out vec4 fragColor;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform vec2 screenResolution;

uniform vec3 camUp;
uniform vec3 camForward;
uniform vec3 camRight;
uniform float camFarPlane;
uniform float camNearPlane;

uniform float FOVdeg;


// ATMOSPHERE SETTINGS
uniform float planetRadius = 1.0;
uniform float atmosphereScale = 1.25;
float atmosphereRadius;

// RENDERING SETTINGS
uniform int numInScatteringPoints;
uniform int numOpticalDepthPoints;
uniform float intensity;
uniform float densityFalloff;

// COLOR
uniform vec3 scatteringCoefficients;


vec2 raySphere (vec3 sphereCentre, float sphereRadius, vec3 rayOrigin, vec3 rayDir) {
    vec3 offset = rayOrigin - sphereCentre;
    const float a = 1; // set to dot(rayDir, rayDir) if rayDir might be unnormalized
    float b = 2 * dot (offset, rayDir);
    float c = dot (offset, offset) - sphereRadius * sphereRadius;

    float discriminant = b * b - 4 * a * c;

    // No intersections: discriminant < 0	1 intersection: discriminant == 0	2 intersections: discriminant > 0
    if (discriminant > 0) {
        float s = sqrt(discriminant);
        float dstToSphereNear = max (0, (-b - s) / (2 * a));
        float dstToSphereFar = (-b + s) / (2 * a);

        if (dstToSphereFar >= 0) {
            return vec2 (dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }
    return vec2(0,0);
}




float densityAtPoint(vec3 samplePoint){
	float heightAbove = distance(samplePoint, atmosphereCentre) - planetRadius;
	float height01 = heightAbove / (atmosphereRadius - planetRadius);
	height01 = clamp(height01, 0.0, 1.0);

	float localDensity = exp(-height01 * densityFalloff) * (1 - height01);

	return localDensity;
}




// Light coming from the sun to the point
float opticalDepth(vec3 rayOrigin, vec3 rayDir, float sunRayLength){
	vec3 densitySamplePoint = rayOrigin;
	float stepSize = sunRayLength / (numOpticalDepthPoints - 1);
	float opticalDepth = 0;

	for(int i = 0; i < numOpticalDepthPoints; i++){
		float localDensity = densityAtPoint(densitySamplePoint);
		opticalDepth += localDensity * stepSize;
		densitySamplePoint += rayDir * stepSize;
	}

	return opticalDepth;
}


// Light coming from the points along the ray
vec3 calculateLight(vec3 rayOrigin, vec3 rayDir, float dstThrough, vec3 originalCol){

	vec3 inScatterPoint = rayOrigin;
	vec3 inScatteredLight;
	
	float stepSize = dstThrough / (numInScatteringPoints - 1);
	float viewRayOpticalDepth = 0.0;

	for(int i = 0; i < numInScatteringPoints; i++){
		vec3 dirToSun = normalize(sunPos - inScatterPoint);
		
		float sunRayLength = raySphere(atmosphereCentre, atmosphereRadius, inScatterPoint, dirToSun).y;
		float sunRayOpticalDepth = opticalDepth(inScatterPoint, dirToSun, sunRayLength);
		viewRayOpticalDepth = opticalDepth(inScatterPoint, -rayDir, stepSize * i);
		vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth) * scatteringCoefficients);
		float localDensity = densityAtPoint(inScatterPoint);

		inScatteredLight += localDensity * transmittance * scatteringCoefficients * stepSize;
		inScatterPoint += rayDir * stepSize;
	}

	inScatteredLight *= intensity;
	float originalColTransmittance = exp(-viewRayOpticalDepth);

	// return originalCol * originalColTransmittance + inScatteredLight;
	return inScatteredLight;
}




float LinearizeDepth(float depth)
{
    float z =  2.0 * depth - 1.0; // Back to NDC
    return (2.0 * camNearPlane * camFarPlane) / (camFarPlane + camNearPlane - z * (camFarPlane - camNearPlane));
}




void main(){
	fragColor = texture(screenTexture, fragCoord);

	vec2 rayCoord = fragCoord * 2.0 - 1.0;

	float fov = radians(FOVdeg); // adjust as needed
	float aspect = screenResolution.x / screenResolution.y;
	float scale = tan(fov * 0.5);

	vec3 rayDir = normalize(	camForward + rayCoord.x * aspect * scale * camRight + rayCoord.y * scale * camUp	);
	vec3 rayOrigin = camPos;

	atmosphereRadius = planetRadius * atmosphereScale;

	vec2 intersect = raySphere(atmosphereCentre, atmosphereRadius, rayOrigin, rayDir); 
	float dstTo = intersect.x;
	float dstThrough = intersect.y;

	float depth;
	depth = raySphere(atmosphereCentre, planetRadius, rayOrigin, rayDir).x - dstTo;
	// depth = texture(depthTexture, fragCoord).r - dstTo;
	
	// DEPTH THING NEEDS TO BE ADDED
	if(depth > 0.0){
		dstThrough = min(dstThrough, depth);
	}


	if(dstThrough > 0.0){
		const float epsilon = 0.001;
		vec3 pointInAtmosphere = rayOrigin + rayDir * (dstTo + epsilon);
		vec3 light = calculateLight(pointInAtmosphere, rayDir, dstThrough - (epsilon * 2), vec3(fragColor));

		fragColor += vec4(light, 1.0);
	}


} 