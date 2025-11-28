#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LENGTH 50
#define MAX_SPELLS 100
#define WIZARD_SPELLS 3

struct Spell {
    char name[MAX_NAME_LENGTH];
    char magic_class[MAX_NAME_LENGTH];
    int min_damage;
    int max_damage;
    int min_mana;
    int max_mana;
};

struct Wizard {
    char name[MAX_NAME_LENGTH];
    char magic_class[MAX_NAME_LENGTH];
    struct Spell spells[WIZARD_SPELLS];
    int hp;
    int mana;
    int alive;
    int total_damage_dealt;
    int total_mana_spent;
    int recoveries;
};

struct Spell spellbook[MAX_SPELLS];
int spell_count = 0;

int load_spells(const char* filename) {
    FILE* file;
    char line[200];
    char* token;
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open %s\n", filename);
        return 0;
    }
    spell_count = 0;
    while (fgets(line, sizeof(line), file) != NULL && spell_count < MAX_SPELLS) {
        int i;
        line[strcspn(line, "\n")] = 0;
        token = strtok(line, ",");
        if (token != NULL) strcpy(spellbook[spell_count].name, token);
        token = strtok(NULL, ",");
        if (token != NULL) strcpy(spellbook[spell_count].magic_class, token);
        token = strtok(NULL, ",");
        if (token != NULL) spellbook[spell_count].min_damage = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL) spellbook[spell_count].max_damage = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL) spellbook[spell_count].min_mana = atoi(token);
        token = strtok(NULL, ",");
        if (token != NULL) spellbook[spell_count].max_mana = atoi(token);
        spell_count++;
    }
    fclose(file);
    return spell_count;
}

void assign_random_spells(struct Wizard* wizard) {
    int i;
    int random_index;
    for (i = 0; i < WIZARD_SPELLS; i++) {
        random_index = rand() % spell_count;
        wizard->spells[i] = spellbook[random_index];
    }
}

int calculate_score(int stats[], int n) {
    int multipliers[3] = {2, 1, -3};
    if (n <= 0) return 0;
    return (stats[n-1] * multipliers[n-1]) + calculate_score(stats, n-1);
}

void duel(struct Wizard* attacker, struct Wizard* defender) {
    int spell_index;
    struct Spell chosen_spell;
    int mana_cost;
    int damage;
    int recovery;

    if (defender->hp <= 0) {
        defender->alive = 0;
        return;
    }
    spell_index = rand() % WIZARD_SPELLS;
    chosen_spell = attacker->spells[spell_index];
    mana_cost = chosen_spell.min_mana + (rand() % (chosen_spell.max_mana - chosen_spell.min_mana + 1));
    damage = chosen_spell.min_damage + (rand() % (chosen_spell.max_damage - chosen_spell.min_damage + 1));
    if (attacker->mana < mana_cost) {
        recovery = 10 + (rand() % 11);
        attacker->mana += recovery;
        attacker->recoveries++;
        printf("%s is low on mana and meditates...\n", attacker->name);
        printf("%s recovers %d mana. Current mana: %d\n", attacker->name, recovery, attacker->mana);
        printf("-------------------------------------------------\n");
        duel(defender, attacker);
        return;
    }
    if (strcmp(attacker->magic_class, chosen_spell.magic_class) == 0) {
        damage += 5;
    }
    defender->hp -= damage;
    attacker->mana -= mana_cost;
    attacker->total_damage_dealt += damage;
    attacker->total_mana_spent += mana_cost;
    printf("%s casts %s on %s!\n", attacker->name, chosen_spell.name, defender->name);
    printf("Damage: %d | %s's HP: %d | %s's Mana: %d\n", damage, defender->name, defender->hp, attacker->name, attacker->mana);
    printf("-------------------------------------------------\n");
    if (defender->hp <= 0) {
        defender->alive = 0;
        return;
    }
    duel(defender, attacker);
}

void print_wizard_summary(struct Wizard* wizard) {
    int stats[3];
    int score;
    stats[0] = wizard->total_damage_dealt;
    stats[1] = wizard->total_mana_spent;
    stats[2] = wizard->recoveries;
    score = calculate_score(stats, 3);
    printf("%s — Damage: %d | Mana Spent: %d | Recoveries: %d | Score: %d\n", wizard->name, wizard->total_damage_dealt, wizard->total_mana_spent, wizard->recoveries, score);
}

int main() {
    struct Wizard wizard1, wizard2;
    char wizard1_name[MAX_NAME_LENGTH];
    char wizard2_name[MAX_NAME_LENGTH];
    srand(time(NULL));
    if (load_spells("spellbook.txt") == 0) {
        printf("Failed to load spells. Creating default spells...\n");
        strcpy(spellbook[0].name, "Fireball");
        strcpy(spellbook[0].magic_class, "fire");
        spellbook[0].min_damage = 10; spellbook[0].max_damage = 25;
        spellbook[0].min_mana = 10; spellbook[0].max_mana = 15;
        strcpy(spellbook[1].name, "Inferno");
        strcpy(spellbook[1].magic_class, "fire");
        spellbook[1].min_damage = 15; spellbook[1].max_damage = 30;
        spellbook[1].min_mana = 15; spellbook[1].max_mana = 20;
        strcpy(spellbook[2].name, "Flame Surge");
        strcpy(spellbook[2].magic_class, "fire");
        spellbook[2].min_damage = 12; spellbook[2].max_damage = 22;
        spellbook[2].min_mana = 12; spellbook[2].max_mana = 18;
        strcpy(spellbook[3].name, "Ice Shard");
        strcpy(spellbook[3].magic_class, "ice");
        spellbook[3].min_damage = 8; spellbook[3].max_damage = 20;
        spellbook[3].min_mana = 9; spellbook[3].max_mana = 14;
        strcpy(spellbook[4].name, "Freeze");
        strcpy(spellbook[4].magic_class, "ice");
        spellbook[4].min_damage = 10; spellbook[4].max_damage = 24;
        spellbook[4].min_mana = 11; spellbook[4].max_mana = 16;
        strcpy(spellbook[5].name, "Blizzard");
        strcpy(spellbook[5].magic_class, "ice");
        spellbook[5].min_damage = 14; spellbook[5].max_damage = 28;
        spellbook[5].min_mana = 14; spellbook[5].max_mana = 20;
        spell_count = 6;
    }
    printf("=== Wizard War Game ===\n");
    printf("Enter first wizard's name: ");
    fgets(wizard1_name, MAX_NAME_LENGTH, stdin);
    wizard1_name[strcspn(wizard1_name, "\n")] = 0;
    printf("Enter second wizard's name: ");
    fgets(wizard2_name, MAX_NAME_LENGTH, stdin);
    wizard2_name[strcspn(wizard2_name, "\n")] = 0;
    strcpy(wizard1.name, wizard1_name);
    if (rand() % 2 == 0)
        strcpy(wizard1.magic_class, "fire");
    else
        strcpy(wizard1.magic_class, "ice");
    wizard1.hp = 100;
    wizard1.mana = 100;
    wizard1.alive = 1;
    wizard1.total_damage_dealt = 0;
    wizard1.total_mana_spent = 0;
    wizard1.recoveries = 0;
    assign_random_spells(&wizard1);

    strcpy(wizard2.name, wizard2_name);
    if (rand() % 2 == 0)
        strcpy(wizard2.magic_class, "fire");
    else
        strcpy(wizard2.magic_class, "ice");
    wizard2.hp = 100;
    wizard2.mana = 100;
    wizard2.alive = 1;
    wizard2.total_damage_dealt = 0;
    wizard2.total_mana_spent = 0;
    wizard2.recoveries = 0;
    assign_random_spells(&wizard2);

    printf("Wizard Duel Begins: %s vs %s!\n", wizard1.name, wizard2.name);
    printf("-------------------------------------------------\n");

    if (rand() % 2 == 0)
        duel(&wizard1, &wizard2);
    else
        duel(&wizard2, &wizard1);

    if (wizard1.alive == 1 && wizard2.alive == 0)
        printf("Winner: %s the %s Wizard!\n", wizard1.name, wizard1.magic_class);
    else if (wizard2.alive == 1 && wizard1.alive == 0)
        printf("Winner: %s the %s Wizard!\n", wizard2.name, wizard2.magic_class);
    else
        printf("It's a tie!\n");

    printf("\nBattle Summary:\n");
    print_wizard_summary(&wizard1);
    print_wizard_summary(&wizard2);

    return 0;
}
