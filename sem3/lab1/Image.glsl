float sdRectangle(vec2 uv, vec2 size, vec2 center) {
    vec2 d = abs(uv - center) - size;
    return max(d.x, d.y);
}

float sdCircle(vec2 uv, float radius, vec2 center) {
    return length(uv - center) - radius;
}

float sdCar(vec2 uv, vec2 carSize, vec2 center) {

    float body = sdRectangle(uv, carSize, center);
    vec2 wheelBarSize = vec2(carSize.x * 1.4, carSize.y * 0.2);
    
    vec2 topWheelBarOffset = center + vec2(0.0, carSize.y * 0.6);
    float topWheelBar = sdRectangle(uv, wheelBarSize, topWheelBarOffset);
    
    vec2 bottomWheelBarOffset = center + vec2(0.0, -carSize.y * 0.6);
    float bottomWheelBar = sdRectangle(uv, wheelBarSize, bottomWheelBarOffset);
    
    float car = min(body, topWheelBar);
    car = min(car, bottomWheelBar);
    
    return car;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    
    vec2 uv = fragCoord / iResolution.xy;
    uv -= 0.5;
    uv.x *= iResolution.x / iResolution.y;
    
    vec2 carPos = vec2(texelFetch(iChannel0, ivec2(0, 0), 0).r, 0.15);
    
    vec2 carOffset = carPos - 0.5;
    carOffset.x *= iResolution.x / iResolution.y;
    
    vec2 carSize = vec2(0.04, 0.08);

    float car = sdCar(uv, carSize, carOffset);

    vec3 carColor = vec3(0.5, 0.5, 0.5);
    vec3 bgColor = vec3(0.5, 1.0, 0.5);
    vec3 roadColor = vec3(0.3, 0.3, 0.3);
    vec3 color = bgColor;
    
    float road = sdRectangle(uv, vec2(0.4, 1.0), vec2(0.0, 0.0));
    
    if (road < 0.0) {
        color = roadColor;
    }
    
    //доп задание сделать текстуру, которая двигается слева-направо
    if (car < 0.0) {
        vec2 carLocalUV = uv - carOffset;
        
        float scrollSpeed = 0.2; 
        carLocalUV.x += iTime * scrollSpeed;
        
        color = texture(iChannel3, carLocalUV).rgb;
    }
    //конец допа
    const int NUM_OBJECTS = 10;
    
    for (int i = 0; i < NUM_OBJECTS; i++) {
    vec4 objectState = texelFetch(iChannel2, ivec2(i, 0), 0);
    vec2 objectPos = objectState.xy;
    float objectType = objectState.z;
    
    if (objectPos.y > -0.2) {
        vec2 objectOffset = objectPos - 0.5;
        objectOffset.x *= iResolution.x / iResolution.y;
        
        float objectSDF;
        if (objectType == 0.0) {
            objectSDF = sdCircle(uv, 0.04, objectOffset);
        } else {
            objectSDF = sdRectangle(uv, vec2(0.04), objectOffset);
        }
        
        if (objectSDF < 0.0) {
            if (objectType == 0.0) {
                color = vec3(1.0, 0.0, 0.0); 
            } else {
                color = vec3(0.0, 0.0, 1.0);
            }
        }
    }
}
    fragColor = vec4(color, 1.0);
}