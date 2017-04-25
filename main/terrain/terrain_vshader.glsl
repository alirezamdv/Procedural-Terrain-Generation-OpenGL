#version 330

uniform vec2 zoomOffset;
uniform float zoom;
uniform sampler2D heightMap;

in vec2 gridPos;

out vec2 uv_TC;
out vec3 vpoint_TC;

void main() {

    //Outputs UV coordinate for fragment shader. Grid coordinates are in [-1, 1] x [-1, 1]
    uv_TC = (gridPos + vec2(1.0, 1.0)) * 0.5;

    float vheight = texture(heightMap, (uv_TC+zoomOffset) * zoom).r;

    //Already sets displacement so we can cull patches that fall outside the view frustrum
    vpoint_TC = vec3(gridPos.x, vheight, -gridPos.y);

}
