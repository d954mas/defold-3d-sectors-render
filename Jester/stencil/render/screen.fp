varying mediump vec4 var_position;
varying mediump vec3 var_normal;
varying mediump vec2 var_texcoord0;
varying mediump vec4 var_light;

uniform mediump vec4 light;

uniform lowp sampler2D tex0;
uniform lowp vec4 tint;

void main()
{
    // Pre-multiply alpha since all runtime textures already are
    vec4 tint_pm = vec4(tint.xyz * tint.w, tint.w);
    vec2 pos = var_texcoord0.xy;
    pos.y = 1.0 - pos.y;
    vec4 color = texture2D(tex0, pos) * tint_pm;
    gl_FragColor = vec4(color.rgb,1.0);
}
