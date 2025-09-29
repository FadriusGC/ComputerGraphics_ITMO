float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    int objectId = int(fragCoord.x);
    
    ivec2 texCoord = ivec2(objectId, int(fragCoord.y));
    vec4 previousState = texelFetch(iChannel2, texCoord, 0);
    
    float posX = previousState.r;
    float posY = previousState.g;
    float objectType = previousState.b;
    float respawnTime = previousState.a;

    if (iFrame == 0) {
        posX = 0.1 + rand(vec2(float(objectId), 1.0)) * 0.8;
        posY = 1.0 + rand(vec2(float(objectId), 2.0)) * 0.5;
        objectType = round(rand(vec2(float(objectId), 3.0)));
        respawnTime = 0.0;
    }

    float speed = 0.4+(0.01*iTime);

    if (posY > -0.2 && respawnTime <= iTime) {
        posY -= speed * iTimeDelta;
        
        if (posY < -0.2) {
            float delay = 0.5 + rand(vec2(float(objectId), iTime)) * 2.5;
            respawnTime = iTime + delay;
            posY = -10.0;
        }
    } 
    else if (respawnTime > 0.0 && iTime >= respawnTime) {
        posX = 0.1 + rand(vec2(float(objectId), iTime)) * 0.8;
        posY = 1.0 + rand(vec2(float(objectId), iTime + 1.0)) * 0.3;
        respawnTime = 0.0;
    }
    
    fragColor = vec4(posX, posY, objectType, respawnTime);
}