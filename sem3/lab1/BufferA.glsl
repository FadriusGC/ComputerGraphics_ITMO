const int KEY_LEFT  = 37;
const int KEY_RIGHT = 39;
const float speed = 1.0;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    float currentX = texelFetch(iChannel0, ivec2(0, 0), 0).r;
    
    if (iFrame == 0) {
        if (int(fragCoord.x) == 0) {
            fragColor = vec4(0.5, 0.0, 0.0, 1.0);
        } else {
            fragColor = vec4(0.0);
        }
        return;
    }
    
    float rightPressed = texelFetch(iChannel1, ivec2(KEY_RIGHT, 0), 0).r;
    float leftPressed  = texelFetch(iChannel1, ivec2(KEY_LEFT,  0), 0).r;
    
    float dx = (iTimeDelta * speed) * (rightPressed - leftPressed);
    float carHalfWidth = 0.04;
    float newX = clamp(currentX + dx, carHalfWidth, 1.0 - carHalfWidth);
    
    if (int(fragCoord.x) == 0) {
        fragColor = vec4(newX, 0.0, 0.0, 1.0);
    } else {
        fragColor = vec4(0.0);
    }
}