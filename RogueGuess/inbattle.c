// inbattle.c
#include <stdio.h>
#include <stdlib.h>   // rand, abs
#include "inbattle.h"

static int used_second_chance_in_run = 0;  // 이번 런에서 두 번째 기회를 썼는지

static int used_second_chance_trait = 0;   // trait로 부활 쓴 적 있는지
static int remaining_rebirth_revives = 0;  // 이번 런에서 남은 영구 부활 횟수


// 전투용 상태바 (전투마다 맨 위에 출력)
static void print_battle_status(const Player* p, const GameState* g) {
    printf("[Lv %d] EXP %d/%d | Gold %d | Rebirth coin %d",
        p->level, p->exp, p->exp_to_next, p->gold, p->rebirth_coin);

    if (g != NULL && g->in_battle) {
        printf(" | Stage %d-%d | HP %d/%d",
            g->stage, g->mob_index, g->hp, g->max_hp);
    }

    printf("\n");
    printf("--------------------------------------------------\n");
}

// 레벨업 체크 (1→2: +50골드, 2→3: +100골드, ...)
static void check_level_up(Player* p) {
    while (p->exp >= p->exp_to_next) {
        p->exp -= p->exp_to_next;
        p->level++;

        int reward = 50 * (p->level - 1);  // 1→2:50, 2→3:100 ...
        p->gold += reward;

        printf("===== LEVEL UP! Lv %d 달성! Gold +%d =====\n",
            p->level, reward);

        // 필요 경험치 조금씩 증가
        p->exp_to_next += 25;
    }
}

// 몬스터 설정 + 체력 trait 반영
static void setup_monster(Player* p, GameState* g, int stage, int mob_index) {
    g->stage = stage;
    g->mob_index = mob_index;
    g->in_battle = 1;

    // --- 스테이지별 기본 몬스터 설정 ---
    if (stage == 1) {
        if (mob_index == 1) {          // 1-1
            g->max_number = 30;
            g->max_hp = 5;
        }
        else if (mob_index == 2) {     // 1-2
            g->max_number = 40;
            g->max_hp = 5;
        }
        else if (mob_index == 3) {     // 1-3 (보스)
            g->max_number = 60;
            g->max_hp = 7;
        }
        else {
            g->max_number = 30;
            g->max_hp = 5;
        }
    }
    else if (stage == 2) {
        if (mob_index == 1) {          // 2-1
            g->max_number = 50;
            g->max_hp = 6;
        }
        else if (mob_index == 2) {     // 2-2
            g->max_number = 70;
            g->max_hp = 7;
        }
        else if (mob_index == 3) {     // 2-3 (보스)
            g->max_number = 90;
            g->max_hp = 9;
        }
        else {
            g->max_number = 50;
            g->max_hp = 6;
        }
    }
    else if (stage == 3) {
        if (mob_index == 1) {          // 3-1
            g->max_number = 80;
            g->max_hp = 8;
        }
        else if (mob_index == 2) {     // 3-2
            g->max_number = 100;
            g->max_hp = 10;
        }
        else if (mob_index == 3) {     // 3-3 (최종 보스)
            g->max_number = 120;
            g->max_hp = 12;
        }
        else {
            g->max_number = 80;
            g->max_hp = 8;
        }
    }
    else {
        // 아직 구현 안 된 스테이지
        g->max_number = 30;
        g->max_hp = 5;
    }

    // 체력 증가 특성 적용
    if (p->trait_hp_level > 0) {
        g->max_hp += p->trait_hp_level;
    }

    g->hp = g->max_hp;

}



// 전투 시작 시 한 번만 아이템 사용 여부
// - 작은 포션: 이번 전투 HP +3
// - 날카로운 직감: 정답이 상/하반부 어느 쪽인지 출력
static void use_items_at_start(Player* p, GameState* g, int answer) {
    int have_any = 0;

    if (p->item_potion_small > 0) have_any = 1;
    if (p->item_insight > 0 || p->trait_insight_level > 0) have_any = 1;

    if (!have_any) {
        printf("사용 가능한 아이템이 없습니다.\n\n");
        return;
    }

    printf("전투 시작! 아이템을 사용하시겠습니까?\n");
    printf("1. 예\n");
    printf("2. 아니오\n");
    printf("선택: ");

    int choice;
    if (scanf_s("%d", &choice) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        return;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    if (choice != 1) {
        printf("아이템을 사용하지 않았습니다.\n\n");
        return;
    }

    int running = 1;
    while (running) {
        printf("\n=== 사용 가능한 아이템 ===\n");
        if (p->item_potion_small > 0)
            printf("1. 작은 포션 (보유: %d개) - 이번 전투 HP +3\n", p->item_potion_small);
        if (p->item_insight > 0 || p->trait_insight_level > 0)
            printf("2. 날카로운 직감 (아이템 또는 특성)\n");
        printf("0. 취소\n");
        printf("선택: ");

        int sel;
        if (scanf_s("%d", &sel) != 1) {
            printf("잘못된 입력입니다.\n");
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        while ((c = getchar()) != '\n' && c != EOF) {}

        if (sel == 0) {
            printf("아이템 사용을 취소했습니다.\n\n");
            running = 0;
        }
        else if (sel == 1) {
            if (p->item_potion_small <= 0) {
                printf("작은 포션이 없습니다.\n");
                continue;
            }
            p->item_potion_small--;
            g->max_hp += 3;
            g->hp += 3;
            printf("작은 포션 사용! 이번 전투에서 HP가 3 증가합니다. (HP: %d/%d)\n\n",
                g->hp, g->max_hp);
            running = 0;
        }
        else if (sel == 2) {
            if (p->item_insight <= 0 && p->trait_insight_level <= 0) {
                printf("날카로운 직감 아이템/특성이 없습니다.\n");
                continue;
            }

            int half = g->max_number / 2;
            if (answer > half) {
                printf("날카로운 직감 발동! 정답은 %d보다 큰 수입니다. (%d ~ %d)\n\n",
                    half, half + 1, g->max_number);
            }
            else {
                printf("날카로운 직감 발동! 정답은 %d 이하의 수입니다. (1 ~ %d)\n\n",
                    half, half);
            }

            if (p->item_insight > 0) {
                p->item_insight--;
            }
            // trait_insight_level > 0 이면 아이템 없이도 사용 가능 느낌

            running = 0;
        }
        else {
            printf("0~2 중에서 선택하세요.\n");
        }
    }
}

// 단일 몬스터와의 전투
// 리턴값: 1 = 플레이어 승리, 0 = 사망(런 종료)
static int fight_monster(Player* p, GameState* g) {
    int answer = (rand() % g->max_number) + 1;

    // 전투 시작 시 한 번만 아이템 사용
    use_items_at_start(p, g, answer);

    int alive = 1;
    int player_win = 0;

    while (alive && g->hp > 0) {
        // 🔹 전투 턴마다 상태바 출력
        print_battle_status(p, g);

        printf("상대: Stage %d-%d 몬스터 (숫자 범위: 1 ~ %d)\n",
            g->stage, g->mob_index, g->max_number);

        int guess;
        printf("1 ~ %d 사이의 숫자를 입력하세요: ", g->max_number);
        if (scanf_s("%d", &guess) != 1) {
            printf("숫자를 입력해야 합니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        if (guess < 1 || guess > g->max_number) {
            printf("범위를 벗어났습니다! (1~%d)\n", g->max_number);
            continue;
        }

        // 🔹 판정 범위 trait 적용
        int bonus_range = 0;
        if (p->trait_range_level > 0) {
            bonus_range += p->trait_range_level;
        }
        if (p->rebirth_range_level > 0) {
            bonus_range += p->rebirth_range_level;
        }

        int diff = abs(guess - answer);
        int is_correct = (diff <= bonus_range);

        if (is_correct) {
            printf("정답! 몬스터를 처치했습니다!\n");

            int exp_gain = 0;
            int gold_gain = 0;

            // --- 스테이지별 기본 보상 ---
            if (g->stage == 1 && g->mob_index == 1) {          // 1-1
                exp_gain = 15;
                gold_gain = 10;
            }
            else if (g->stage == 1 && g->mob_index == 2) {     // 1-2
                exp_gain = 20;
                gold_gain = 15;
            }
            else if (g->stage == 1 && g->mob_index == 3) {     // 1-3 (보스)
                exp_gain = 40;
                gold_gain = 30;
            }
            else if (g->stage == 2 && g->mob_index == 1) {     // 2-1
                exp_gain = 30;
                gold_gain = 25;
            }
            else if (g->stage == 2 && g->mob_index == 2) {     // 2-2
                exp_gain = 40;
                gold_gain = 30;
            }
            else if (g->stage == 2 && g->mob_index == 3) {     // 2-3 (보스)
                exp_gain = 70;
                gold_gain = 50;
            }
            else if (g->stage == 3 && g->mob_index == 1) {     // 3-1
                exp_gain = 60;
                gold_gain = 40;
            }
            else if (g->stage == 3 && g->mob_index == 2) {     // 3-2
                exp_gain = 80;
                gold_gain = 60;
            }
            else if (g->stage == 3 && g->mob_index == 3) {     // 3-3 (최종 보스)
                exp_gain = 120;
                gold_gain = 100;
            }
            else {
                // 기타 기본값
                exp_gain = 10;
                gold_gain = 5;
            }

            // 골드 보너스 특성
            if (p->trait_gold_bonus_level > 0) {
                gold_gain += p->trait_gold_bonus_level * 10;
            }

            // 정확한 감각 특성: 보상 2배
            if (p->trait_exact_level > 0) {
                exp_gain *= 2;
                gold_gain *= 2;
                printf("정확한 감각 발동! 보상이 2배로 증가했습니다.\n");
            }

            p->exp += exp_gain;
            p->gold += gold_gain;

            printf("EXP +%d, Gold +%d 획득!\n", exp_gain, gold_gain);

            check_level_up(p);

            player_win = 1;
            alive = 0;
        }


        else {
            g->hp--;

            if (guess < answer) {
                printf("더 큰 수입니다!\n");
            }
            else {
                printf("더 작은 수입니다!\n");
            }

            printf("HP가 1 감소했습니다. (현재 HP: %d/%d)\n",
                g->hp, g->max_hp);

            if (g->hp <= 0) {
                // 1순위: 아이템 부활
                if (p->item_second_chance > 0) {
                    p->item_second_chance--;
                    printf("두 번째 기회 '아이템'이 발동했습니다!\n");
                    g->hp = g->max_hp;
                    printf("HP가 전부 회복되고 전투를 계속합니다! (HP: %d/%d)\n",
                        g->hp, g->max_hp);
                    continue;
                }

                // 2순위: 일반 특성 부활 (한 번만)
                if (p->trait_second_chance_level > 0 && !used_second_chance_trait) {
                    used_second_chance_trait = 1;
                    printf("두 번째 기회 '특성'이 발동했습니다!\n");
                    g->hp = g->max_hp;
                    printf("HP가 전부 회복되고 전투를 계속합니다! (HP: %d/%d)\n",
                        g->hp, g->max_hp);
                    continue;
                }

                // 3순위: 영구 패시브 부활 (레벨만큼 여러 번)
                if (remaining_rebirth_revives > 0) {
                    remaining_rebirth_revives--;
                    printf("영구 두 번째 기회가 발동했습니다! (남은 영구 부활: %d회)\n",
                        remaining_rebirth_revives);
                    g->hp = g->max_hp;
                    printf("HP가 전부 회복되고 전투를 계속합니다! (HP: %d/%d)\n",
                        g->hp, g->max_hp);
                    continue;
                }

                // 진짜로 사망
                printf("체력이 0이 되어 쓰러졌습니다...\n");
                printf("이번 런은 여기서 종료됩니다.\n");
                alive = 0;
                player_win = 0;
            }

        }

        printf("\n");
    }

    g->in_battle = 0;
    return player_win;
}

// 외부에서 호출하는 런 시작 함수
// 스테이지 하나(1 or 2)를 도는 함수
static int run_stage(Player* p, GameState* g, int stage) {
    printf("%d 스테이지를 시작합니다!\n\n", stage);

    // 1~3번 몹(2마리 + 보스) 순서대로
    for (int mob = 1; mob <= 3; mob++) {
        setup_monster(p, g, stage, mob);

        if (!fight_monster(p, g)) {
            printf("%d-%d 에서 패배했습니다...\n\n", stage, mob);
            return 0;  // 죽으면 런 종료
        }

        if (mob < 3)
            printf("%d-%d 스테이지 클리어!\n\n", stage, mob);
        else
            printf("%d 스테이지 보스를 처치했습니다!\n\n", stage);
    }

    printf("%d 스테이지를 모두 클리어했습니다!\n\n", stage);
    return 1;
}

// 외부에서 호출하는 런 시작 함수
void start_new_game(Player* p, GameState* g) {
    printf("새 게임을 시작합니다! (1 스테이지부터 진행)\n\n");

    used_second_chance_trait = 0;
    remaining_rebirth_revives = p->rebirth_second_chance_level;  // 영구 부활 횟수 세팅

    // 1스테이지
    if (!run_stage(p, g, 1)) {
        return;
    }

    // 2스테이지
    if (!run_stage(p, g, 2)) {
        return;
    }

    // 여기 오면 2스테이지까지 모두 클리어
    // 👉 여기서 환생 여부 물어보기
    // (run_stage(3)는 더 이상 호출 안 함)

    printf("게임을 모두 클리어하셨습니다.\n");
    printf("환생하시겠습니까?\n");
    printf("1. 예\n");
    printf("2. 아니요\n");
    printf("선택: ");

    int choice;
    if (scanf_s("%d", &choice) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        return;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}

    if (choice == 1) {
        // 환생
        p->rebirth_coin += 1;
        printf("환생 코인을 1개 얻었습니다! (현재: %d개)\n", p->rebirth_coin);
        printf("환생합니다... 레벨, 경험치, 골드, 아이템, 일반 특성이 초기화됩니다.\n");

        // 여기서 "런마다 초기화되는 것들"만 리셋
        p->level = 1;
        p->exp = 0;
        p->exp_to_next = 50;
        p->gold = 0;

        p->item_potion_small = 0;
        p->item_insight = 0;
        p->item_second_chance = 0;

        p->trait_hp_level = 0;
        p->trait_range_level = 0;
        p->trait_gold_bonus_level = 0;
        p->trait_exact_level = 0;
        p->trait_insight_level = 0;
        p->trait_second_chance_level = 0;

        // 영구 패시브(rebirth_*), rebirth_coin은 그대로 유지
    }
    else {
        printf("환생하지 않고 메인 메뉴로 돌아갑니다.\n");
    }
}


