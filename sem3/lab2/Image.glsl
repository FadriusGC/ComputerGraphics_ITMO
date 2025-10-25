const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 100.0;
const float PRECISION = 0.001;

struct Surface {
    float sd;
    vec3 col;
    int objType;
};

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

float sdBox(vec3 p, vec3 size) {
    vec3 q = abs(p) - size;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sdSphere(vec3 p, float radius) {
    return length(p) - radius;
}

float sdPlane(vec3 p, float height) {
    return p.y - height;
}

Surface minWithColor(Surface a, Surface b) {
    if (a.sd < b.sd) return a;
    return b;
}

Surface sdTank(vec3 p, vec3 tankPos, vec3 color, bool isEnemy, float turretAngle, float barrelAngle) {
    Surface result = Surface(MAX_DIST, vec3(0), -1);
    
    vec3 localPos = p - tankPos;
    
    //корпус
    float hull = sdBox(localPos, vec3(1.0, 0.4, 1.8));
    result = minWithColor(result, Surface(hull, color, 1));
    
    float enemyTurretAngle = 0.0;
    float enemyBarrelAngle = 0.0;
    
    //башня
    vec3 turretLocal = localPos;
    if (isEnemy) {
        turretLocal = turretLocal * rotateY(enemyTurretAngle);
    } else {
        turretLocal = turretLocal * rotateY(turretAngle);
    }
    float turret = sdBox(turretLocal - vec3(0, 0.7, 0), vec3(0.6, 0.3, 0.7));
    result = minWithColor(result, Surface(turret, color * 0.8, 2));
    
    //ствол
    if (isEnemy) {
        //типо вражеский танк
        vec3 barrelLocal = localPos;
        barrelLocal = barrelLocal * rotateY(enemyTurretAngle);
        
        vec3 barrelBase = vec3(0, 0.7, 0.5);
        vec3 barrelVector = vec3(0, 0, 1.0);
        barrelVector = rotateX(enemyBarrelAngle) * barrelVector;
        
        vec3 barrelCenter = barrelBase + barrelVector * 0.5;
        vec3 barrelPos = barrelLocal - barrelCenter;
        barrelPos = rotateX(enemyBarrelAngle) * barrelPos;
        
        float barrel = sdBox(barrelPos, vec3(0.07, 0.07, 0.5));
        result = minWithColor(result, Surface(barrel, color * 0.6, 3));
    } else {
        //типо наш танк
        vec3 barrelLocal = localPos;
        barrelLocal = barrelLocal * rotateY(turretAngle);
        
        vec3 barrelBase = vec3(0, 0.7, 0.5);
        vec3 barrelVector = vec3(0, 0, 1.0);
        barrelVector = rotateX(barrelAngle) * barrelVector;
        
        vec3 barrelCenter = barrelBase + barrelVector * 0.5;
        vec3 barrelPos = barrelLocal - barrelCenter;
        barrelPos = rotateX(-barrelAngle) * barrelPos;
        
        float barrel = sdBox(barrelPos, vec3(0.07, 0.07, 0.5));
        result = minWithColor(result, Surface(barrel, color * 0.6, 3));
    }
    
    return result;
}

Surface sdGround(vec3 p) {
    float ground = sdPlane(p, -1.0);
    vec3 grassColor = vec3(0.2, 0.6, 0.1);
    float pattern = sin(p.x * 5.0) * cos(p.z * 5.0) * 0.1;
    grassColor += vec3(pattern * 0.2);
    
    // Обработка воронок
    const int NUM_CRATERS = 20;
    for (int i = 0; i < NUM_CRATERS; i++) {
        vec4 crater = texelFetch(iChannel3, ivec2(i, 0), 0);
        if (crater.w > 0.5) { // активная воронка
            float radius = crater.z;
            float circleDist = length(vec2(p.x - crater.x, p.z - crater.y)) - radius;
            if (circleDist < 0.0) {
                ground = max(ground, circleDist);
                grassColor = vec3(0.1, 0.1, 0.1); 
            }
        }
    }
    
    return Surface(ground, grassColor, 0);
}

Surface sdShell(vec3 p, vec3 shellPos, bool shellActive) {
    if (!shellActive) return Surface(MAX_DIST, vec3(0), -1);
    
    float shell = sdSphere(p - shellPos, 0.08);
    return Surface(shell, vec3(1.0, 0.8, 0.0), 4);
}

Surface sdBomb(vec3 p, vec4 bomb) {
    float bombRadius = 0.08;
    vec3 bombPos = vec3(bomb.x, bomb.y, bomb.z);
    float bombSd = sdSphere(p - bombPos, bombRadius);
    return Surface(bombSd, vec3(1.0, 0.0, 0.0), 5);
}

Surface sdScene(vec3 p, float turretAngle, float barrelAngle, vec3 shellPos, bool shellActive) {
    Surface scene = sdGround(p);
    scene = minWithColor(scene, sdTank(p, vec3(-3, -0.6, 0), vec3(0.3, 0.3, 0.8), false, turretAngle, barrelAngle));
    scene = minWithColor(scene, sdTank(p, vec3(3, -0.6, 0), vec3(0.8, 0.3, 0.3), true, turretAngle, barrelAngle));
    scene = minWithColor(scene, sdShell(p, shellPos, shellActive));
    
    // Добавляем бомбы
    const int NUM_BOMBS = 5;
    for (int i = 0; i < NUM_BOMBS; i++) {
        vec4 bomb = texelFetch(iChannel2, ivec2(i, 0), 0);
        if (bomb.w == 1.0) { // падает
            Surface bombSurf = sdBomb(p, bomb);
            scene = minWithColor(scene, bombSurf);
        }
    }
    return scene;
}

Surface rayMarch(vec3 ro, vec3 rd, float start, float end, float turretAngle, float barrelAngle, vec3 shellPos, bool shellActive) {
    float depth = start;
    Surface co;
    
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 p = ro + depth * rd;
        co = sdScene(p, turretAngle, barrelAngle, shellPos, shellActive);
        depth += co.sd;
        if (co.sd < PRECISION || depth > end) break;
    }
    
    co.sd = depth;
    return co;
}

vec3 calcNormal(vec3 p, float turretAngle, float barrelAngle, vec3 shellPos, bool shellActive) {
    float eps = 0.001;
    
    vec3 dx = vec3(eps, 0, 0);
    vec3 dy = vec3(0, eps, 0); 
    vec3 dz = vec3(0, 0, eps);
    
    //считаем градиент, матан убивает
    float gradX = sdScene(p + dx, turretAngle, barrelAngle, shellPos, shellActive).sd 
                - sdScene(p - dx, turretAngle, barrelAngle, shellPos, shellActive).sd;
    
    float gradY = sdScene(p + dy, turretAngle, barrelAngle, shellPos, shellActive).sd
                - sdScene(p - dy, turretAngle, barrelAngle, shellPos, shellActive).sd;
    
    float gradZ = sdScene(p + dz, turretAngle, barrelAngle, shellPos, shellActive).sd
                - sdScene(p - dz, turretAngle, barrelAngle, shellPos, shellActive).sd;
    
    return normalize(vec3(gradX, gradY, gradZ));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec4 keyState = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 angleState = texelFetch(iChannel0, ivec2(2, 0), 0);
    vec4 shellState = texelFetch(iChannel1, ivec2(0, 0), 0);
    
    float turretAngle = angleState.x;
    float barrelAngle = angleState.y;
    vec3 shellPos = shellState.xyz;
    bool shellActive = shellState.w > 0.5;
    
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    vec3 ro = vec3(0, 2, 8);
    vec3 rd = normalize(vec3(uv, -1));
    
    vec3 backgroundColor = vec3(0.5, 0.7, 1.0);
    
    Surface co = rayMarch(ro, rd, MIN_DIST, MAX_DIST, turretAngle, barrelAngle, shellPos, shellActive);
    
    vec3 color = backgroundColor;
    
    if (co.sd < MAX_DIST) {
        vec3 p = ro + rd * co.sd;
        vec3 normal = calcNormal(p, turretAngle, barrelAngle, shellPos, shellActive);
        
        vec3 lightDir = normalize(vec3(1, 2, 1));
        float diff = max(dot(normal, lightDir), 0.0);
        
        vec3 material = co.col;
        if (co.objType == 0) {
            material = co.col * diff;
        } else if (co.objType == 4) {
            material = co.col * (diff + 0.3);
        }
        
        color = material;
        
        vec3 shadowRo = p + normal * 0.1;
        Surface shadowCo = rayMarch(shadowRo, lightDir, 0.01, 5.0, turretAngle, barrelAngle, shellPos, shellActive);
        if (shadowCo.sd < 5.0) {
            color *= 0.5;
        }
    }
    
    fragColor = vec4(color, 1.0);
}