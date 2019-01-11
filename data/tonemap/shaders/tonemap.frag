#version 460 core

#define HORIZONTAL_SPLIT_4 0
#define QUAD_SPLIT_4       1
#define LINEAR_ONLY        2
#define FILMIC_LUT_ONLY    3
#define ACES_ONLY          4
#define FILMIC_UC2_ONLY 5

uniform int view;

layout(binding = 0) uniform sampler2D s;
layout(binding = 1) uniform sampler3D lut_sampler;

in VS_OUT
{
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;

//
// ACES
//

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// original by Stephen Hill

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = transpose(ACESInputMat) * color; // transpose because HLSL matrix

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = transpose(ACESOutputMat) * color;

    // Clamp to [0, 1]
    color = clamp(color, vec3(0), vec3(1));

    return color;
}

//
// Filmic
//

/*  // https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/53
    A = shoulder strength
    B = linear strength
    C = Linear Angle
    D = Toe Strength
    E = Toe Numerator
    F = Toe Denominator
    => E/F = Toe Angle

    F(x) = ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
    FinalColor = F(LinearColor)/F(LinearWhite)

    // without pow(1/2.2)
    A = 0.22
    B = 0.30
    C = 0.10
    D = 0.20
    E = 0.01
    F = 0.30
    Linear White Point Value = 11.2
*/

// includes pow(1/2.2)
vec3 Filmic_1(vec3 linear_hdr)
{
    vec3 x = max(vec3(0), linear_hdr - vec3(0.004));
    return (x*(6.2*x+0.5))/(x*(6.2*x+1.7)+0.06);
}

vec3 Linear_To_sRGB(vec3 linear_color)
{
    return pow(linear_color, vec3(1.0/2.2));
}

vec3 lut(vec3 linear_hdr)
{
    float white_point = 4.0;
    // scale down and clamp input colors to fit the LUT texcoords.
    vec3 lut_tc = min((1.0/white_point)*linear_hdr, vec3(1));
    
    return texture(lut_sampler, lut_tc).rgb;
}


void main()
{
    vec3 linear_hdr = texture(s, fs_in.tc).rgb;
    switch(view)
    {
        case HORIZONTAL_SPLIT_4: 
        {
            if(fs_in.tc.x < 0.25)
            {
                // CLAMP
                vec3 c = clamp(linear_hdr, vec3(0), vec3(1));
                outColor = vec4(c, 1);
            }
            else if(fs_in.tc.x < 0.5)
            {
                // Filmic LUT
                outColor = vec4(lut(linear_hdr), 1);
            }
            else if(fs_in.tc.x < 0.75)
            {
                // ACES + GAMMA
                vec3 c = ACESFitted(linear_hdr);
                outColor = vec4(Linear_To_sRGB(c), 1);
            }
            else
            {
                // FILMIC + GAMMA
                outColor = vec4(Filmic_1(linear_hdr), 1);
            }
        } break;

        case QUAD_SPLIT_4: break;

        case LINEAR_ONLY: 
        {
            vec3 c = clamp(linear_hdr, vec3(0), vec3(1));
            outColor = vec4(c, 1);
        } break;

        case FILMIC_LUT_ONLY: 
        {
            outColor = vec4(lut(linear_hdr), 1);
        } break;

        case ACES_ONLY: 
        {
            vec3 c = ACESFitted(linear_hdr);
            outColor = vec4(Linear_To_sRGB(c), 1);
        } break;

        case FILMIC_UC2_ONLY: 
        {
            outColor = vec4(Filmic_1(linear_hdr), 1);
        } break;
    }



    
}
