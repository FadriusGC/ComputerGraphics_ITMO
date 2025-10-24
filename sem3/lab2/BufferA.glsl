const int KEY_A = 65;
const int KEY_D = 68;
const int KEY_W = 87;
const int KEY_S = 83;
const int KEY_SPACE = 32;

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    //первый кадр
    if (iFrame == 0) {
        if (fragCoord.x < 1.0 && fragCoord.y < 1.0) {
            fragColor = vec4(0.0); // A, D, W, S
        }
        else if (fragCoord.x < 2.0 && fragCoord.y < 1.0) {
            fragColor = vec4(0.0, 0.0, 0.0, 0.0); // space states
        }
        else if (fragCoord.x < 3.0 && fragCoord.y < 1.0) {
            fragColor = vec4(0.0, 0.0, 0.0, 0.0); // turretAngle, barrelAngle
        }
        else {
            fragColor = vec4(0.0);
        }
        return;
    }
    
    //читка предыдщуего состояния
    vec4 prevKeys = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 prevSpace = texelFetch(iChannel0, ivec2(1, 0), 0);
    vec4 prevAngles = texelFetch(iChannel0, ivec2(2, 0), 0);
    
    //текущее состояние клавиш
    float keyA = texelFetch(iChannel1, ivec2(KEY_A, 0), 0).r;
    float keyD = texelFetch(iChannel1, ivec2(KEY_D, 0), 0).r;
    float keyW = texelFetch(iChannel1, ivec2(KEY_W, 0), 0).r;
    float keyS = texelFetch(iChannel1, ivec2(KEY_S, 0), 0).r;
    float keySpace = texelFetch(iChannel1, ivec2(KEY_SPACE, 0), 0).r;
    
    float turretAngle = prevAngles.x;
    float barrelAngle = prevAngles.y;
    
    //вращаем все то, что можно вращать
    turretAngle += (keyD - keyA) * 0.05;
    barrelAngle += (keyW - keyS) * 0.03;
    barrelAngle = clamp(barrelAngle, -0.3, 0.4);
    
    //выстрел T-T
    float spacePressed = 0.0;
    if (keySpace > 0.5 && prevSpace.y < 0.5) {
        spacePressed = 1.0;
    }
    
    //сохраняемся
    if (fragCoord.x < 1.0 && fragCoord.y < 1.0) {
        fragColor = vec4(keyA, keyD, keyW, keyS);
    }
    else if (fragCoord.x < 2.0 && fragCoord.y < 1.0) {
        fragColor = vec4(keySpace, prevSpace.x, spacePressed, 0.0);
    }
    else if (fragCoord.x < 3.0 && fragCoord.y < 1.0) {
        fragColor = vec4(turretAngle, barrelAngle, 0.0, 0.0);
    }
    else {
        fragColor = vec4(0.0);
    }
}