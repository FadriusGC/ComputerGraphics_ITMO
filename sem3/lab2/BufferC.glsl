const int NUM_BOMBS = 5;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    if (fragCoord.x >= float(NUM_BOMBS)) {
        fragColor = vec4(0.0);
        return;
    }

    int bombIndex = int(fragCoord.x);
    vec4 prevState = texelFetch(iChannel2, ivec2(bombIndex, 0), 0);

    if (iFrame == 0) {
        float respawnTime = rand(vec2(float(bombIndex), 1.0)) * 5.0;
        fragColor = vec4(0.0, respawnTime, 0.0, 0.0); // x, respawnTime, z, state
        return;
    }

    float x = prevState.x;
    float y = prevState.y;
    float z = prevState.z;
    float state = prevState.w;

    // state: 0.0 = неактивна, 1.0 = падает, 2.0 = упала
    if (state == 0.0) { 
        float respawnTime = y;
        if (iTime >= respawnTime) {
            float newX = rand(vec2(float(bombIndex), iTime)) * 10.0 - 5.0;
            float newZ = rand(vec2(float(bombIndex), iTime + 1.0)) * 10.0 - 5.0;
            float newY = 10.0;
            fragColor = vec4(newX, newY, newZ, 1.0); // переход в состояние падения
        } else {
            fragColor = prevState;
        }
    } else if (state == 1.0) { 
        y -= 2.5 * iTimeDelta;
        if (y < -0.5) {
            fragColor = vec4(x, y, z, 2.0);
        } else {
            fragColor = vec4(x, y, z, 1.0);
        }
    } else if (state == 2.0) {
        fragColor = vec4(x, y, z, 0.0); // сбрасываем в неактивное состояние
    }
}