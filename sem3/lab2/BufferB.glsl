const bool SHELL_DISAPPEAR_ON_HIT = true;

mat3 rotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0), 
        vec3(-s, 0, c)
    );
}

mat3 rotateX(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    if (iFrame == 0) {
        if (fragCoord.x < 1.0 && fragCoord.y < 1.0) {
            fragColor = vec4(0.0, 100.0, 0.0, 0.0);
        }
        else if (fragCoord.x < 2.0 && fragCoord.y < 1.0) {
            fragColor = vec4(0.0);
        }
        else {
            fragColor = vec4(0.0);
        }
        return;
    }
    
    vec4 spaceState = texelFetch(iChannel0, ivec2(1, 0), 0);
    vec4 angleState = texelFetch(iChannel0, ivec2(2, 0), 0);
    
    vec4 prevShellState = texelFetch(iChannel1, ivec2(0, 0), 0);
    vec4 prevShellVel = texelFetch(iChannel1, ivec2(1, 0), 0);
    
    float turretAngle = angleState.x;
    float barrelAngle = angleState.y;
    bool spacePressed = (spaceState.z > 0.5);
    
    vec3 shellPos = prevShellState.xyz;
    bool shellActive = prevShellState.w > 0.5;
    vec3 shellVel = prevShellVel.xyz;
    
    if (spacePressed) {
        shellActive = true;
        
        vec3 tankPos = vec3(-3, -0.6, 0);
        vec3 turretOffset = vec3(0, 0.8, 0);
        
        //место вылета снарядов
        vec3 barrelBaseLocal = vec3(0, -0.1, 0.5);
        float barrelLength = 1.0;
        
        //направление от основания к концу
        vec3 barrelVector = vec3(0, 0, barrelLength);
        
        //применяем наклон ствола
        barrelVector = rotateX(barrelAngle) * barrelVector;
        
        //применяем провороты башни
        barrelBaseLocal = rotateY(turretAngle) * barrelBaseLocal;
        barrelVector = rotateY(turretAngle) * barrelVector;
        
        shellPos = tankPos + turretOffset + barrelBaseLocal + barrelVector;
        
        vec3 shotDirection = normalize(barrelVector);
        
        shellVel = shotDirection * 0.15;
        
        prevShellState = vec4(shellPos, 1.0);
        prevShellVel = vec4(shellVel, 0.0);
    }
    
    if (shellActive) {
        shellPos += shellVel * 0.8;
        shellVel.y -= 0.006;
        
        if (shellPos.y < -1.0) {
            if (SHELL_DISAPPEAR_ON_HIT) {
                shellActive = false;
                prevShellState.w = 0.0;
            } else {
                shellPos.y = -1.0 + 0.08;
                shellVel = vec3(0);
            }
        }
        
        if (length(shellPos - vec3(-3, -0.6, 0)) > 25.0) {
            shellActive = false;
            prevShellState.w = 0.0;
        }
        
        if (shellActive) {
            prevShellState.xyz = shellPos;
            prevShellVel.xyz = shellVel;
        }
    }
    
    if (fragCoord.x < 1.0 && fragCoord.y < 1.0) {
        fragColor = prevShellState;
    }
    else if (fragCoord.x < 2.0 && fragCoord.y < 1.0) {
        fragColor = prevShellVel;
    }
    else {
        fragColor = vec4(0.0);
    }
}