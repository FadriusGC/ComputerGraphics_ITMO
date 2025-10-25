const int NUM_BOMBS = 5;
const int NUM_CRATERS = 20; /
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    if (fragCoord.x >= float(NUM_CRATERS)) {
        fragColor = vec4(0.0);
        return;
    }

    int craterIndex = int(fragCoord.x);
    vec4 crater = texelFetch(iChannel3, ivec2(craterIndex, 0), 0);
    
    fragColor = crater;
    
    // Ищем бомбы, которые только что упали (state == 2.0)
    for (int i = 0; i < NUM_BOMBS; i++) {
        vec4 bomb = texelFetch(iChannel2, ivec2(i, 0), 0);
        
        // Проверяем, что бомба только что упала (state == 2.0)
        if (bomb.w == 2.0) {
            // Ищем первый свободный слот в буфере воронок
            bool isFreeSlot = (crater.w <= 0.5);
            bool isEarliestFree = true;
            
            for (int j = 0; j < craterIndex; j++) {
                vec4 prevCrater = texelFetch(iChannel3, ivec2(j, 0), 0);
                if (prevCrater.w <= 0.5) {
                    isEarliestFree = false;
                    break;
                }
            }
            
            // Если это первый свободный слот, создаем новую воронку
            if (isFreeSlot && isEarliestFree) {
                fragColor = vec4(bomb.x, bomb.z, 0.5, 1.0); // x, z, radius, active
            }
        }
    }
}