// fxaa.fs
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;

#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

void main() {
    vec2 texelSize = 1.0 / resolution;
    vec2 frameBufSize = resolution;
    
    // Próbkowanie pikseli
    vec3 rgbNW = texture(texture0, fragTexCoord + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 rgbNE = texture(texture0, fragTexCoord + vec2(1.0, -1.0) * texelSize).rgb;
    vec3 rgbSW = texture(texture0, fragTexCoord + vec2(-1.0, 1.0) * texelSize).rgb;
    vec3 rgbSE = texture(texture0, fragTexCoord + vec2(1.0, 1.0) * texelSize).rgb;
    vec3 rgbM  = texture(texture0, fragTexCoord).rgb;

    // Obliczanie luminancji
    const vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    // Obliczanie maksymalnej i minimalnej luminancji
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    // Obliczanie kierunku gradientu
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    // Normalizacja i skalowanie kierunku
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX),
              dir * rcpDirMin)) * texelSize;

    // Zaawansowane próbkowanie wzdłuż kierunku gradientu
    vec3 rgbA = 0.5 * (
        texture(texture0, fragTexCoord + dir * (1.0/3.0 - 0.5)).rgb +
        texture(texture0, fragTexCoord + dir * (2.0/3.0 - 0.5)).rgb);
        
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(texture0, fragTexCoord + dir * -0.5).rgb +
        texture(texture0, fragTexCoord + dir * 0.5).rgb);
        
    float lumaB = dot(rgbB, luma);

    // Wybór końcowego koloru
    finalColor = vec4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
}