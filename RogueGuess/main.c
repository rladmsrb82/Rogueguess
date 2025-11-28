// main.c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "inbattle.h"
#include "cheat.h"

// 상태창 출력 (메인 메뉴용)
void print_status(const Player* p, const GameState* g) {
    printf("[Lv %d] EXP %d/%d | Gold %d | Rebirth coin %d",
        p->level, p->exp, p->exp_to_next, p->gold, p->rebirth_coin);

    if (g != NULL && g->in_battle) {
        printf(" | Stage %d-%d | HP %d/%d",
            g->stage, g->mob_index, g->hp, g->max_hp);
    }

    printf("\n");
    printf("--------------------------------------------------\n");
}

void show_menu() {
    printf("===== ROGUEGUESS =====\n");
    printf("1. 새 게임\n");
    printf("2. 이어하기\n");
    printf("3. 게임 방법\n");
    printf("4. 상점\n");
    printf("5. 게임 종료\n");
    printf("6. 환생 상점\n");      // 🔹 이 줄 추가
    printf("======================\n");
    printf("선택: ");
}


// 새 게임용 플레이어 초기화
void reset_player(Player* p) {

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

    // rebirth_hp_level, rebirth_range_level, rebirth_second_chance_level, rebirth_coin 유지
}


// 세이브 / 로드
int save_game(const Player* p) {
    FILE* fp = NULL;
    if (fopen_s(&fp, "save.dat", "wb") != 0 || fp == NULL) {
        printf("세이브 파일 저장에 실패했습니다.\n");
        return 0;
    }

    size_t written = fwrite(p, sizeof(Player), 1, fp);
    fclose(fp);

    if (written != 1) {
        printf("세이브 파일을 쓰는 데 실패했습니다.\n");
        return 0;
    }

    printf("[AUTO SAVE] 게임이 저장되었습니다.\n");
    return 1;
}

int load_game(Player* p) {
    FILE* fp = NULL;
    if (fopen_s(&fp, "save.dat", "rb") != 0 || fp == NULL) {
        return 0;
    }

    size_t read = fread(p, sizeof(Player), 1, fp);
    fclose(fp);

    if (read != 1) {
        printf("세이브 파일을 읽는 데 실패했습니다.\n");
        return 0;
    }

    return 1;
}

void continue_game(Player* p, GameState* g) {
    if (load_game(p)) {
        printf("세이브 데이터를 불러왔습니다.\n");
        print_status(p, g);

        // 불러온 상태로 1스테이지 런 1번 진행
        start_new_game(p, g);

        // 런 종료 후 현재 상태 저장
        save_game(p);
    }
    else {
        printf("저장된 게임이 없습니다.\n");
    }
}

void show_help() {
    printf("=== 게임 방법 ===\n");
    printf("- 1~N 사이 숫자를 맞추면 몬스터를 처치합니다.\n");
    printf("- HP(Health)는 시도 횟수와 같고, 0이 되면 이번 런이 종료됩니다.\n");
    printf("- 보스를 잡으면 특성을 얻고, 런을 마치면 상점에서 아이템을 구매할 수 있습니다.\n");
    printf("- 새 게임은 무조건 처음부터, 이어하기는 저장된 상태에서 다음 런을 진행합니다.\n");
    printf("==================\n");
}

// 상점
void open_shop(Player* p, GameState* g) {
    int running = 1;

    while (running) {
        print_status(p, g);
        printf("===== 상점 =====\n");
        printf("1. 작은 포션 (+체력 회복용, 전투 시작 시 사용) - 20 Gold\n");
        printf("2. 날카로운 직감 스크롤 (힌트용) - 30 Gold\n");
        printf("3. 두 번째 기회 토큰 (한 번 부활) - 100 Gold\n");
        printf("0. 나가기\n");
        printf("================\n");
        printf("선택: ");

        int choice;
        if (scanf_s("%d", &choice) != 1) {
            printf("잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        if (choice == 0) {
            printf("상점을 나갑니다.\n");
            break;
        }

        int bought = 0;

        switch (choice) {
        case 1:
            if (p->gold >= 20) {
                p->gold -= 20;
                p->item_potion_small++;
                printf("작은 포션을 구매했습니다! (보유: %d개)\n", p->item_potion_small);
                bought = 1;
            }
            else {
                printf("골드가 부족합니다.\n");
            }
            break;
        case 2:
            if (p->gold >= 30) {
                p->gold -= 30;
                p->item_insight++;
                printf("날카로운 직감 스크롤을 구매했습니다! (보유: %d개)\n", p->item_insight);
                bought = 1;
            }
            else {
                printf("골드가 부족합니다.\n");
            }
            break;
        case 3:
            if (p->gold >= 100) {
                p->gold -= 100;
                p->item_second_chance++;
                printf("두 번째 기회 토큰을 구매했습니다! (보유: %d개)\n", p->item_second_chance);
                bought = 1;
            }
            else {
                printf("골드가 부족합니다.\n");
            }
            break;
        default:
            printf("0~3 중에서 선택하세요.\n");
            break;
        }

        // 상점에서 무언가를 샀다면 즉시 저장
        if (bought) {
            save_game(p);
        }

        printf("\n");
    }
}

void open_rebirth_shop(Player* p) {
    int running = 1;

    while (running) {
        printf("===== 환생 상점 =====\n");
        printf("현재 환생 코인: %d개\n", p->rebirth_coin);
        printf("1. 영구 HP +1 (현재 레벨: %d / 최대 3)\n", p->rebirth_hp_level);
        printf("2. 영구 판정 범위 +1 (현재 레벨: %d / 최대 3)\n", p->rebirth_range_level);
        printf("3. 영구 두 번째 기회 +1 (현재 레벨: %d / 최대 3)\n", p->rebirth_second_chance_level);
        printf("0. 나가기\n");
        printf("=====================\n");
        printf("선택: ");

        int choice;
        if (scanf_s("%d", &choice) != 1) {
            printf("잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        if (choice == 0) {
            printf("환생 상점을 나갑니다.\n");
            break;
        }

        int* level_ptr = NULL;
        const char* name = NULL;

        if (choice == 1) {
            level_ptr = &p->rebirth_hp_level;
            name = "영구 HP";
        }
        else if (choice == 2) {
            level_ptr = &p->rebirth_range_level;
            name = "영구 판정 범위";
        }
        else if (choice == 3) {
            level_ptr = &p->rebirth_second_chance_level;
            name = "영구 두 번째 기회";
        }
        else {
            printf("0~3 중에서 선택하세요.\n\n");
            continue;
        }

        if (*level_ptr >= 3) {
            printf("%s는 이미 최대 레벨입니다.\n\n", name);
            continue;
        }

        int next_level = *level_ptr + 1;
        int cost = next_level;   // 1레벨: 1코인, 2레벨: 2코인, 3레벨: 3코인

        if (p->rebirth_coin < cost) {
            printf("환생 코인이 부족합니다. (필요: %d개)\n\n", cost);
            continue;
        }

        p->rebirth_coin -= cost;
        *level_ptr = next_level;

        printf("%s 레벨이 %d로 증가했습니다! (소모 코인: %d개, 남은 코인: %d개)\n\n",
            name, *level_ptr, cost, p->rebirth_coin);

        // 영구 패시브 바뀌었으니 바로 저장
        save_game(p);
    }
}


// 메인 루프
void game_loop(Player* player, GameState* game) {
    int running = 1;

    while (running) {
        print_status(player, game);
        show_menu();

        int choice;
        if (scanf_s("%d", &choice) != 1) {
            printf("잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        switch (choice) {
        case 1:
            reset_player(player);
            start_new_game(player, game);
            save_game(player);
            break;
        case 2:
            continue_game(player, game);
            break;
        case 3:
            show_help();
            break;
        case 4:
            open_shop(player, game);
            break;
        case 5:
            save_game(player);
            printf("게임을 종료합니다.\n");
            running = 0;
            break;
        case 6:   // 🔹 환생 상점
            open_rebirth_shop(player);
            break;
        case 9:
            open_cheat_menu(player);
            save_game(player);
            break;
        default:
            printf("1~6을 선택하거나, 개발자 코드 9를 입력하세요.\n");
            break;
        }


        printf("\n");
    }
}

int main(void) {
    Player player = { 0 };
    GameState game = { 0 };

    // 처음 실행 시는 새 게임 기준 상태
    reset_player(&player);
    srand((unsigned int)time(NULL));
    game.in_battle = 0;

    game_loop(&player, &game);
    return 0;
}
