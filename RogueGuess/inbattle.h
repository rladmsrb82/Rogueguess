// inbattle.h
#ifndef INBATTLE_H
#define INBATTLE_H

// 플레이어 정보
typedef struct {
    int level;
    int exp;
    int exp_to_next;
    int gold;
    int rebirth_coin;

    // 아이템
    int item_potion_small;
    int item_insight;
    int item_second_chance;

    // 일반 특성 (이번 런 동안만)
    int trait_hp_level;
    int trait_range_level;
    int trait_gold_bonus_level;
    int trait_exact_level;
    int trait_insight_level;
    int trait_second_chance_level;

    // 🔹 영구 패시브 (환생 상점에서 사는 것들)
    int rebirth_hp_level;             // 영구 HP 보너스 (0~3)
    int rebirth_range_level;          // 영구 판정 범위 보너스 (0~3)
    int rebirth_second_chance_level;  // 영구 부활 횟수 (0~3, 런당)
} Player;


// 전투/스테이지 상태
typedef struct {
    int stage;       // 스테이지 번호
    int mob_index;   // 1,2 = 잡몹, 3 = 보스
    int hp;          // 현재 HP
    int max_hp;      // 최대 HP
    int max_number;  // 숫자 범위 (1 ~ max_number)
    int in_battle;   // 전투 중이면 1, 아니면 0
} GameState;

// 1스테이지 런 시작 (1-1 → 1-2 → 1-3 보스)
void start_new_game(Player* p, GameState* g);

#endif
