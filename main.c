#include <stdio.h>
#include <gb/gb.h>
#include <gb/metasprites.h>

#include "Inv_Anim.c"

#include "Dudeb.c"
#include "enemytiles.c"

#include "bktiles.c"

#include "player.c"
#include "gumba.c"

#include "maps.h"
#include "heart.c"
#include "spieler.h"
#include "stretchAttack.h"
#include "agent.h"

#include <gb/font.h>

#define GRAV 1 
#define P_ACCEL 1 
#define Y_RAGD 2 
#define X_RAGD 2

#define BKOMAX 10
#define ENEMYMAX 10


extern const int largeLevel_objectcount;
extern int largeLevelobjects[][2];
extern const int largeLevel_enemycount;
extern int largeLevelenemies[][3];

int bkobject_x[BKOMAX];
int bkobject_width[BKOMAX];
int bkobject_y[BKOMAX];
int bkobject_height[BKOMAX];

int enemyType[ENEMYMAX];
int enemyPose[ENEMYMAX];
int enemyHp[ENEMYMAX];
int enemyX[ENEMYMAX];
int enemyXlast[ENEMYMAX];
int enemyY[ENEMYMAX];
int enemyYlast[ENEMYMAX];
int enemy_spawnLoc[ENEMYMAX];
int enemySpawned[ENEMYMAX];
int enemyXspeed[ENEMYMAX];
int enemyYspeed[ENEMYMAX];
int enemyDir[ENEMYMAX];
int enemyWalkcounter[ENEMYMAX];
int enemyHitcounter[ENEMYMAX];
int enemyDeathcounter[ENEMYMAX];
int enemyRelation[ENEMYMAX];
int enemyBkcoll[ENEMYMAX];


void level(int objectCount);
const int maps2Count = 2;


int bkgXScroll = 0, bkgYScroll = 0, bkgXlast = 0;
int bkgX, drawn, drawnLast;

void performantdelay(int cycles);

int collision(int x, int width, int y, int height, int x2, int width2, int y2, int height2);

//Background stuff
void setup_bkobject(int maxObjects, int levelArray[][2]);
void blockEntity_collision(int x, int width, int y, int height, int *entX, int entXlast, int entXempty, int entWidth, int *entY, int entYlast, int entHeight, int *entBkcoll);
void draw(void);

//Player stuff
void moveplayer(int x, int y);
void playergrav(void);
void pragdoll(void);
void basicmove(void);
void playerHp(void);
void invulnerable(void);
void normalatt(void);


//Enemy stuff
void setup_enemy(int enemyCount, int enemyArray[][3]);
void updateEnemies(int enemyCount);
void moveEnemies(void);


int bpress;

void main(void)
{
    set_bkg_data(0, 6, bktiles);
    set_win_data(6, 2, heart);

    setup_bkobject(largeLevel_objectcount, largeLevelobjects);
    setup_enemy(largeLevel_enemycount, largeLevelenemies);
    set_bkg_submap(0, 0, 32, 18, largemap, 100);

    SHOW_BKG;
    DISPLAY_ON;
    move_bkg(0, 0);
    bkgX = 0;
    drawn = 0;

    

    //set_sprite_data(0, 33, dudeb);
    set_sprite_data(1, sizeof(spieler_data) >> 4, spieler_data);
    pose = 0;
    set_sprite_data(13, 10, stretchAttack);
    moveplayer(pX, pY);
    xlast = pX;
    ylast = pY;

    //set_sprite_data(33, 2, enemy);
    set_sprite_data(35, 5, invl);
    set_sprite_data(40, sizeof(agent_data) >> 4, agent_data);
    playerHp();
   
    SHOW_SPRITES;
    while(pHp != 0)
    {          
        if(joypad() & J_B)
        {
            if(attcounter == 0 && bpress == 0)
            {
                attcounter = 1;
            }
            bpress = 1;
        }
        else
        {
            bpress = 0;
        }  
        if(pragdoll_counter == 0)
        {
            basicmove();
            playergrav();
            normalatt();
        }
        else if (invul == 0)
        {
            pragdoll();
            playergrav();
            attcounter = 0;
        }
        
        
        if (invul > 0)
        {
            invulnerable();
        }
                
        updateEnemies(3);
        bkgXlast = bkgX;
        level(2);        
        //adress of the enemy struct, tile number, the speeds it will move when attacked jX-jY, and ai move speed
        //gumbamove(&gumba1, 4, -3, 1, 120, 140, 130);
        //gumbamove(&gumba2, 4, -3, 1, 50, 70, 100);        
        if (pX < 100)
        {
            if (pX > 40)
            {
                xlast = pX;
            }
            else
            {
                if(bkgX > 0)
                {
                    bkgXScroll = pX - xlast; 
                    bkgX += bkgXScroll;
                    //PROBLEMATIC !!!!!!!
                    //CHANGE LATER !!!!!
                    pX = 40;   
                    xlast = pX;
                }
                else if(pX > 0)
                {
                    xlast = pX;
                }  
               
            }
        }
        else
        {           
            bkgXScroll = pX - xlast;
            bkgX += bkgXScroll;
            //PROBLEMATIC !!!!!!!
            //CHANGE LATER !!!!!
            pX = 100;   
            xlast = pX;
        }
        ylast = pY;
        //MOVE PLAYER to the location determined as a result of this cycle
        moveplayer(xlast, ylast);
        moveEnemies();
        
        draw();
        performantdelay(3);
        scroll_bkg(bkgXScroll, bkgYScroll);
    }
}




void performantdelay(int cycles)
{
    for(int i = 0; i < cycles; i++)
    {
        wait_vbl_done();
    }
}

void draw(void)
{
    int diff = bkgX/8 - drawn;
    if(diff > 0)
    {
        set_bkg_submap(drawn + 20, 0, 1, 18, largemap, 100);
    }
    else if(diff < 0)
    {
        set_bkg_submap(drawn + diff, 0, -diff, 18, largemap, 100);
    }  
    drawn += diff;
}

//the main collision detection function
int collision(int x, int width, int y, int height, int x2, int width2, int y2, int height2)
{
//EITHER sq1's x[0] is between sq2's x's OR sq1's x[1] is between sq2's x's OR (sq1 x0 is smaller AND sq1 x1 is bigger) // meaning sq1 contains sq2 x's inside
    if((x >= x2 && x <= x2 + width2) || 
    (x + width >= x2 && x + width <= x2 + width2) || 
    (x <= x2 && x + width >= x2 + width2) == 1)
    {
// 1 = passed both, 0 = passed x but failed y        
        return(y >= y2 && y <= y2 + height2) || 
        (y + height >= y2 && y + height <= y2 + height2) || 
        (y <= y2 && y + height >= y2 + height2);
    }
//returns 2 if passes x critera but fails y    
    return 2;
}



void setup_enemy(int enemyCount, int enemyArray[][3])
{
    for(int i = 0; i < enemyCount; i++)
    {    
        //set_sprite_tile(i + 10, 0);
        enemyType[i] = enemyArray[0][i];
        enemyHp[i] = enemyArray[1][i];
        enemy_spawnLoc[i] = enemyArray[2][i];
        enemyX[i] = 0;
        enemyY[i] = enemyArray[3][i];
        enemyYlast[i] = enemyY[i];
        enemyDir[i] = enemyArray[4][i];

        //move_sprite(i + 10, enemyX[i], enemyY[i]);
    
        enemySpawned[i] = 0;
        enemyXspeed[i] = 0;
        enemyYspeed[i] = 0;
        enemyHitcounter[i] = 0;
        enemyDeathcounter[i] = 0;
        enemyRelation[i] = 0;
    }
}


void setup_bkobject(int maxObjects, int levelArray[][2])
{
    for(int i = 0; i < maxObjects; i++)
    {
        bkobject_x[i] = levelArray[0][i];
        bkobject_width[i] = levelArray[1][i];
        bkobject_y[i] = levelArray[2][i];
        bkobject_height[i] = levelArray[3][i];
    }
}

 void level(int objectCount)
{
    pBkcoll = 0;
    for(int i = 0; i < largeLevel_enemycount; i++)
    {
        enemyBkcoll[i] = 0;
    }
    //BKG scroll is inverted
    
    for(int i = 0; i < objectCount; i++)
    {      
        bkobject_x[i] -= bkgXScroll;
        blockEntity_collision(bkobject_x[i], bkobject_width[i], bkobject_y[i], bkobject_height[i], &pX, xlast, pXempty, pWidth, &pY, ylast, pHeight, &pBkcoll);
        for(int c = 0; c < largeLevel_enemycount; c++)
        {
            blockEntity_collision(bkobject_x[i], bkobject_width[i], bkobject_y[i], bkobject_height[i], &enemyX[c], enemyXlast[c], 0, gumbaWidth, &enemyY[c], enemyYlast[c], gumbaHeight, &enemyBkcoll[c]);
        }
    }  
    bkgXScroll = 0;
}

//Regular Block Collision logic
void blockEntity_collision(int x, int width, int y, int height, int *entX, int entXlast, int entXempty, int entWidth, int *entY, int entYlast, int entHeight, int *entBkcoll)
{
    int cond1 = 0;
    int cond2 = 0;
    if((x >= *entX + entXempty && x <= *entX + entXempty + entWidth) || 
    (x + width >= *entX + entXempty && x + width <= *entX + entXempty + entWidth) || 
    (x <= *entX + entXempty && x + width >= *entX + entXempty + entWidth) == 1)
    {      
        if((y > entYlast && y <= entYlast + entHeight) || 
        (y + height >= entYlast && y + height <= entYlast + entHeight) || 
        (y <= entYlast && y + height >= entYlast + entHeight) == 1)
        {
            cond1 = 1;
            if(entXlast <= x)
            {
                *entX = x - entXempty - entWidth -2;
            }
            else if(entXlast > x)
            {
                *entX = x + width - entXempty + 2;
            }
        }
    }       
    if((x >= entXlast + entXempty && x <= entXlast + entXempty + entWidth) || 
    (x + width >= entXlast + entXempty && x + width <= entXlast + entXempty + entWidth) || 
    (x <= entXlast + entXempty && x + width >= entXlast + entXempty + entWidth) == 1)
    {     
        if((y >= *entY && y - height <= *entY ) || 
        (y >= *entY - entHeight && y - height <= *entY - entHeight) || 
        (y >= *entY && y - height <= *entY - entHeight) == 1)
        {
            cond2 = 1;
            if(entYlast <= y - height)
            {
                *entY = y - height;
                *entBkcoll = 1;
            }
            else if(entYlast - entHeight>= y)
            {
                *entY = y + entHeight;
            }
        }
        //the condition for standing/walking on the object       
        else if(*entY == y - height)
        {   
            cond2 = 1;
            *entBkcoll = 1;
        }
    }
    //if neither of the conditions are met, it might be a corner
    if(cond1 != 1 && cond2 != 1)
    {
        if((x >= *entX + entXempty && x <= *entX + entXempty + entWidth) || 
        (x + width >= *entX + entXempty && x + width <= *entX + entXempty + entWidth) || 
        (x <= *entX + entXempty && x + width >= *entX + entXempty + entWidth) == 1)
        {
            if((y >= *entY && y - height <= *entY ) || 
            (y >= *entY - entHeight && y - height <= *entY - entHeight) || 
            (y >= *entY && y - height <= *entY - entHeight) == 1)
            {
                if(entXlast <= x)
                {
                    *entX = x - entXempty - entWidth -2;
                }
                else if(entXlast > x)
                {
                    *entX = x + width - entXempty + 2;
                }
            }
        }
    }
     

}
 
 void playergrav(void)
 {
     if(pBkcoll != 1)
        {
            //this condition is necessary to stop character from going back to jump pose during attack          
            if(attcounter == 0)
            {
                pose = 2;             
            }
            yspeed += GRAV;
            pY += yspeed;                     
        }
        else
        {
            yspeed = 0;
        }
 }

 void pragdoll(void)
 {
     switch(pragdoll_counter)
     {
         case 1:
            yspeed = -Y_RAGD;
            pY += yspeed;
            flingdir = -touching;
            pX += X_RAGD * flingdir;
            pragdoll_counter++;
            break;
         default:
            pX += X_RAGD * flingdir;
            pragdoll_counter++;
            break;
         case 4:
             pragdoll_counter = 0;
             touching = 0;
             invul++;
             break;
     }
 }

  void basicmove(void)
{
        //for jumping   
        if (joypad() & J_A && pBkcoll == 1)
        {
            pBkcoll = 0;
            yspeed = -12;
            pY += yspeed;
        } 
        
        if (joypad() & J_RIGHT)
        {
            //makes it so you cant change your direction once you've already finished the first part of attack animation, but you can still move the other way           
            if (attcounter < 4)
            {
                //used to reset animation counter if it was counting for the other direction previously                
                if (dir == -1)
                {
                    dir = 1;
                    counter = 0;
                }
                if (counter > 2 && counter < 4)
                {
                    pose = 1;
                }
                else if (counter == 4)
                {
                    counter = 0;
                    pose = 0;
                }         
                counter++;
            }
            if (xspeed < speedmax)
                {
                    xspeed += P_ACCEL;
                }    
            
        }
        else if (joypad() & J_LEFT)
        {
            if (attcounter < 4)
            {
                //used to reset animation counter if it was counting for the other direction previously                  
                if (dir == 1)
                {
                    dir = -1;
                    counter = 0;
                }
                if (counter > 2 && counter < 4)
                {
                    pose = 1;
                }
                else if (counter == 4)
                {
                    pose = 0;
                    counter = 0;
                    //set_sprite_tile(0, 3);
                } 
                  
                counter++;
            }
            if (xspeed > -speedmax)
                {
                    xspeed += -P_ACCEL;
                }  
            
        }
        else if (xspeed > 0)
        {
            xspeed += -P_ACCEL;
            //this condition is necessary to stop the character from going to default pose during attack if no button is pressed            
            if (attcounter == 0)
            {
                pose = 0;
                //set_sprite_tile(0, 1);
            }
        }
        else if (xspeed < 0)
        {
            xspeed += P_ACCEL;
            //this condition is necessary to stop the character from going to default pose during attack if no button is pressed            
            if (attcounter == 0)
            {
                pose = 0;
                //set_sprite_tile(0, 3);
            }
        }
        pX += xspeed;         
        
}

void moveplayer(int x, int y)
{    
    //Death condition
    if(y > 160)
    {
        pHp = 0;
    }
    
    if(dir == -1)
    {
        move_metasprite(spieler_metasprites[pose], 1, 0, x, y);
    }
    else
    {
        move_metasprite_vflip(spieler_metasprites[pose], 1, 0, x, y);
    }
}

void playerHp(void)
{
    for(int i = 0; i < pHp; i++)
    {
        set_win_tiles(20, 40, 1, 1, heart);
    }
}

void invulnerable(void)
{
    switch(invul)
    {
        case 1:
            set_sprite_tile(8, 35);
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 2:
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 3:
            set_sprite_tile(8, 36);
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 4:
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 5:
            set_sprite_tile(8, 37);
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 6:
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 7:
            set_sprite_tile(8, 38);
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 8:
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 9:
            set_sprite_tile(8, 39);
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 10:
            move_sprite(8, pX, pY);
            invul++;
            break;
        case 11:
            invul = 0;
            set_sprite_tile(8, 0);
            break;   
    }   
}

void normalatt(void)
{
    if (dir == 1)
            {
                //Right attack animation
                switch(attcounter)
                {
                    case 0:
                        break;
                    case 1:
                        pose = 3;
                        attcounter++;
                        break;
                    case 2:
                        pose = 4;
                        set_sprite_tile(4, 13);
                        move_sprite(4, pX, pY - 16);
                        attcounter++;
                        break;
                    case 3:
                        set_sprite_tile(4, 22);
                        set_sprite_tile(5, 13);
                        move_sprite(4, pX, pY - 16);
                        move_sprite(5, pX, pY - 24);
                        attcounter++;
                        break;
                    case 4: 
                        pose = 5;
                        pAttack[0] = 24;
                        pAttack[1] = -24;
                        set_sprite_tile(4, 15);
                        set_sprite_tile(5, 17);
                        set_sprite_tile(6, 16);
                        set_sprite_tile(7, 18);
                        move_sprite(4, pX + 8, pY - 8);
                        move_sprite(5, pX + 16, pY - 8);
                        move_sprite(6, pX + 8, pY);
                        move_sprite(7, pX + 16, pY);
                        attcounter++;
                        break;
                    //Locks the character in frame till the last case
                    default:
                        move_sprite(4, pX + 8, pY - 8);
                        move_sprite(5, pX + 16, pY - 8);
                        move_sprite(6, pX + 8, pY);
                        move_sprite(7, pX + 16, pY);
                        attcounter++;
                        break;
//determines how many cycles player will be locked in last part of animation                    
                    case 7:
                        
                        pose = 0;
                        pAttack[0] = 0;
                        pAttack[1] = 0;
                        for(int i = 4; i < 8; i++) shadow_OAM[i].y = 0;

                        attcounter = 0;
                        break;
                } 
                
            }
            else if (dir == -1)
            {
                //Left attack animation
                switch(attcounter)
                {
                    case 0:
                        break;
                    case 1:
                        pose = 3;
                        attcounter++;
                        break;
                    case 2:
                        pose = 4;
                        set_sprite_tile(4, 14);
                        move_sprite(4, pX - 8, pY - 16);
                        attcounter++;
                        break;
                    case 3:
                        set_sprite_tile(4, 21);
                        set_sprite_tile(5, 14);
                        move_sprite(4, pX - 8, pY - 16);
                        move_sprite(5, pX - 8, pY - 24);
                        attcounter++;
                        break;
                    case 4:
                        pose = 5;
                        pAttack[0] = -24;
                        pAttack[1] = -24;
                        set_sprite_tile(4, 15);
                        set_sprite_tile(5, 19);
                        set_sprite_tile(6, 16);
                        set_sprite_tile(7, 20);
                        move_sprite(4, pX - 16, pY - 8);
                        move_sprite(5, pX - 24, pY - 8);
                        move_sprite(6, pX - 16, pY);
                        move_sprite(7, pX - 24, pY);
                        attcounter++;
                        break;
                    //Locks the character in frame till the last case
                    default:
                        move_sprite(4, pX - 16, pY - 8);
                        move_sprite(5, pX - 24, pY - 8);
                        move_sprite(6, pX - 16, pY);
                        move_sprite(7, pX - 24, pY);
                        attcounter++;
                        break;
//determines how many cycles player will be locked in last part of animation                    
                    case 7:
                    //after attack ends, sets hitscan area to 0 
                        
                        //set_sprite_tile(0, 1);
                        pose = 0;
                        pAttack[0] = 0;
                        pAttack[1] = 0;
                        for(int i = 4; i < 8; i++) shadow_OAM[i].y = 0;

                        attcounter = 0;
                        break;
                } 
            }
}

void updateEnemies(int enemyCount)
{
    for(int i = 0; i < enemyCount; i++)
    {
        if(enemySpawned[i] == 1)
        {
            switch(enemyType[i])
            {
                case 0:
                    //1 is to the right of player -1 is to the left
                    if(enemyX[i] > pX) enemyRelation[i] = 1;
                    else enemyRelation[i] = -1;

                    if(enemyRelation[i] == 1 && enemyBkcoll[i] == 1 && enemyHitcounter[i] == 0 && enemyDeathcounter[i] == 0)
                    {
                        enemyX[i] -= 1;
                        enemyWalkcounter[i]++;
                    }
                    else if(enemyBkcoll[i] == 1 && enemyHitcounter[i] == 0 && enemyDeathcounter[i] == 0)
                    {
                        enemyX[i] += 1;
                        enemyWalkcounter[i]++;
                    }
                    if(enemyWalkcounter[i] > 5) enemyWalkcounter[i] = 0;

                    //Player Attack Collision
                    if(attcounter > 3)
                    {
                        if(collision(pX, pAttack[0], pY, pAttack[1], enemyX[i], 8, enemyY[i], 8) == 1 && enemyHitcounter[i] == 0)
                        {
                            enemyXspeed[i] = gumbaXfling * enemyRelation[i];
                            enemyYspeed[i] = gumbaYfling;
                            enemyX[i] += enemyXspeed[i];
                            enemyY[i] += enemyYspeed[i];
                            enemyHitcounter[i]++;
                            enemyWalkcounter[i] = 0;
                            enemyHp[i]--;
                        }
                    }
                   
                    if(enemyHitcounter[i] > 0 && enemyHitcounter[i] < 16)
                    {
                        enemyHitcounter[i]++;
                    }
                    else if(enemyHitcounter[i] == 16)
                    {
                        enemyHitcounter[i] = 0;
                    }
                    
                    //Direct Player Collision
                    if (collision(pX, pWidth, pY, pHeight, enemyX[i], 8, enemyY[i], 8) == 1 && pragdoll_counter == 0 && invul == 0)
                    {
                        touching = enemyRelation[i];
                        set_sprite_tile(4, 0);
                        set_sprite_tile(5, 0);
                        set_sprite_tile(6, 0);
                        set_sprite_tile(7, 0);
                        pragdoll_counter++;
                        pHp--;
                    }              
                    
                    if (enemyBkcoll[i] == 0)
                    {
                        enemyYspeed[i] += GRAV;
                        enemyY[i] += enemyYspeed[i];
                        enemyX[i] += enemyXspeed[i];
                    }
                    //!!
                    //hitcounter argument necessary to prevent from reseting speed right after attack,
                    //since the collision will be detected on the next cycle
                    //!!
                    else if(enemyBkcoll[i] == 1 && enemyHitcounter[i] == 0)
                    {
                        enemyXspeed[i] = 0;
                        enemyYspeed[i] = 0;
                    }
                    enemyX[i] -= bkgXScroll;
                                       
                    //Despawns enemy if screen is out of enemyX range 
                    if(enemyX[i] < 0 || enemyX[i] > 160)
                    {
                        enemySpawned[i] = 0;
                        hide_metasprite(agent_metasprites[0], 10 + (i * 4));
                    }
                    //or if hp is 0
                    else if(enemyHp[i] == 0)
                    {
                        enemyXspeed[i] = 0;
                        enemyYspeed[i] = 0;
                        enemyDeathcounter[i]++;
                    }
                    break;
            }
        }
        else
        {
            //Spawns enemy if the screen is in range of spawn point
            if(enemy_spawnLoc[i] >= bkgX && enemy_spawnLoc[i] <= bkgX + 160)
            {
                if(enemy_spawnLoc[i] < bkgXlast || enemy_spawnLoc[i] > bkgXlast + 160)
                {   
                    enemySpawned[i] = 1;
                    enemyX[i] = enemy_spawnLoc[i] - bkgX;
                    enemyXlast[i] = enemyX[i];
                    enemyXspeed[i] = 0;
                    enemyYspeed[i] = 0;
                    enemyWalkcounter[i] = 0;
                    move_metasprite(agent_metasprites[0], 40, 10 + (i * 4), enemyX[i], enemyY[i]);
                }
            }
        }
    }
}


void moveEnemies(void)
{
    for(int i = 0; i < largeLevel_enemycount; i++)
        {
            switch(enemyType[i])
            {
                case 0:
                    if(enemySpawned[i] == 1)
                    {
                        enemyXlast[i] = enemyX[i];
                        enemyYlast[i] = enemyY[i];
                        if(enemyDeathcounter[i] > 0)
                        {
                            move_metasprite(agent_metasprites[4], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i] - enemyDeathcounter[i]*1);
                            enemyDeathcounter[i]++;
                            if(enemyDeathcounter[i] > 15)
                            {
                                enemyHitcounter[i] = 0;
                                enemyDeathcounter[i] = 0;
                                hide_metasprite(agent_metasprites[0], 10 + (i * 4));
                                enemySpawned[i] = 0;
                                enemyHp[i] = 3;
                            }
                        }
                        else if(enemyHitcounter[i] > 0 && enemyHitcounter[i] < 8)
                        {
                            move_metasprite(agent_metasprites[3], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]);
                        }
                        else if(enemyBkcoll[i] == 0 || enemyHitcounter[i] > 0)
                        {
                            move_metasprite(agent_metasprites[0], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]);
                        }
                        else if(enemyWalkcounter[i] > 2)
                        {
                            enemyRelation[i] == 1 ? move_metasprite(agent_metasprites[2], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]): move_metasprite_vflip(agent_metasprites[2], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]);
                        }
                        else
                        {
                            enemyRelation[i] == 1 ? move_metasprite(agent_metasprites[1], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]): move_metasprite_vflip(agent_metasprites[1], 40, 10 + (i * 4), enemyXlast[i], enemyYlast[i]);
                        }
                    }
            }        
        }
}
