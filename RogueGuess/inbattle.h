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
    int item_potion_small;     // 작은 포션 개수
    int item_insight;          // 날카로운 직감 아이템 개수
    int item_second_chance;    // 두 번째 기회 아이템 개수

    // 특성(traits)
    int trait_hp_level;            // 체력 증가 (0~4)
    int trait_range_level;         // 정답 판정 범위 확대 (0~3 → ±0~3)
    int trait_gold_bonus_level;    // 골드 보너스 (0~3 → +0,10,20,30)
    int trait_exact_level;         // 정확한 감각 (0/1 → 보상 2배)
    int trait_insight_level;       // 날카로운 직감 패시브 (0/1)
    int trait_second_chance_level; // 두 번째 기회 패시브 (0/1)
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
