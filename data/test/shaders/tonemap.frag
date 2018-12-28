#version 460 core

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// original by Stephen Hill

layout(binding = 0) uniform sampler2D s;

in VS_OUT
{
    vec2 tc;
} fs_in;

layout(location = 0) out vec4 outColor;




//
// ACES
//

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
    color = transpose(ACESInputMat) * color; // ou dans l'autre sens, le mul
    //color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = transpose(ACESOutputMat) * color; // ou dans l'autre sens, le mul
    //color = color * ACESOutputMat; // ou dans l'autre sens, le mul

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

vec3 tonemap(vec3 linear_hdr)
{
    //return ACESFitted(linear_hdr);
    return Filmic_1(linear_hdr);
}

void main()
{
    vec3 linear_hdr = texture(s, fs_in.tc).rgb;
//    if(fs_in.tc.x < 0.25)
//    {
//        outColor = vec4(linear_hdr, 1);
//    }
//    else if(fs_in.tc.x < 0.5)
//    {
//        outColor = vec4(ACESFitted(linear_hdr), 1);
//    }
//    else if(fs_in.tc.x < 0.75)
//    {
//        vec3 c = ACESFitted(linear_hdr);
//        outColor = vec4(Linear_To_sRGB(c), 1);
//    }
//    else
//    {
//        outColor = vec4(Filmic_1(linear_hdr), 1);
//    }
    vec3 sRGB_ldr = tonemap(linear_hdr);
    outColor = vec4(sRGB_ldr, 1);
}
