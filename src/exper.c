/* NetHack 3.7	exper.c	$NHDT-Date: 1596498167 2020/08/03 23:42:47 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.43 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2007. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifndef LONG_MAX
#include <limits.h>
#endif

/* Return the experience points cutoff to level up to the next level.
 * Oddly, the experience point values are offset by one from the levels they
 * actually represent - newuexp(1) returns how many points are required to
 * achieve experience level 2, etc. */
long
newuexp(int lev)
{
    /* keep this synced with the status-drawing code in the clients */
    switch (lev) {
    case  0: return      0;
    case  1: return     20; /* n^2 */
    case  2: return     40;
    case  3: return     80;
    case  4: return    160;
    case  5: return    320;
    case  6: return    640;
    case  7: return   1280;
    case  8: return   2560;
    case  9: return   5120;
    case 10: return  10000; /* triangle numbers */
    case 11: return  15000;
    case 12: return  21000;
    case 13: return  28000;
    case 14: return  36000;
    case 15: return  45000;
    case 16: return  55000;
    case 17: return  66000;
    case 18: return  81000; /* n*n series */
    case 19: return 100000;
    case 20: return 142000;
    case 21: return 188000;
    case 22: return 238000;
    case 23: return 292000;
    case 24: return 350000;
    case 25: return 412000;
    case 26: return 478000;
    case 27: return 548000;
    case 28: return 622000;
    case 29: return 700000;
    case 30: return 800000; /* 100k per additional !oGL */
    }
    impossible("unknown level: %d", lev);
    return 10000000;
}

/* calculate hp max increase for new level */
int
newhp(void)
{
    int hp, conplus;

    if (u.ulevel == 0) {
        /* Initialize hit points */
        hp = g.urole.hpadv.infix + g.urace.hpadv.infix;
        if (g.urole.hpadv.inrnd > 0)
            hp += rnd(g.urole.hpadv.inrnd);
        if (g.urace.hpadv.inrnd > 0)
            hp += rnd(g.urace.hpadv.inrnd);
        if (g.moves <= 1L) { /* initial hero; skip for polyself to new man */
            /* Initialize alignment stuff */
            u.ualign.type = aligns[flags.initalign].value;
            u.ualign.record = g.urole.initrecord;
        }
        /* no Con adjustment for initial hit points */
    } else {
        if (u.ulevel < ROLE_XLEV_CUTOFF) {
            hp = g.urole.hpadv.lofix + g.urace.hpadv.lofix;
            if (g.urole.hpadv.lornd > 0)
                hp += rnd(g.urole.hpadv.lornd);
            if (g.urace.hpadv.lornd > 0)
                hp += rnd(g.urace.hpadv.lornd);
        } else {
            hp = g.urole.hpadv.hifix + g.urace.hpadv.hifix;
            if (g.urole.hpadv.hirnd > 0)
                hp += rnd(g.urole.hpadv.hirnd);
            if (g.urace.hpadv.hirnd > 0)
                hp += rnd(g.urace.hpadv.hirnd);
        }
        if (ACURR(A_CON) <= 3)
            conplus = -2;
        else if (ACURR(A_CON) <= 6)
            conplus = -1;
        else if (ACURR(A_CON) <= 14)
            conplus = 0;
        else if (ACURR(A_CON) <= 16)
            conplus = 1;
        else if (ACURR(A_CON) == 17)
            conplus = 2;
        else if (ACURR(A_CON) == 18)
            conplus = 3;
        else
            conplus = 4;
        hp += conplus;
    }
    if (hp <= 0)
        hp = 1;
    if (u.ulevel < MAXULEV)
        u.uhpinc[u.ulevel] = (xchar) hp;
    return hp;
}

/* calculate spell power/energy points for new level */
int
newpw(void)
{
    int en = 0, enrnd, enfix;

    if (u.ulevel == 0) {
        en = g.urole.enadv.infix + g.urace.enadv.infix;
        if (g.urole.enadv.inrnd > 0)
            en += rnd(g.urole.enadv.inrnd);
        if (g.urace.enadv.inrnd > 0)
            en += rnd(g.urace.enadv.inrnd);
    } else {
        enrnd = (int) ACURR(A_WIS) / 2;
        if (u.ulevel < ROLE_XLEV_CUTOFF) {
            enrnd += g.urole.enadv.lornd + g.urace.enadv.lornd;
            enfix = g.urole.enadv.lofix + g.urace.enadv.lofix;
        } else {
            enrnd += g.urole.enadv.hirnd + g.urace.enadv.hirnd;
            enfix = g.urole.enadv.hifix + g.urace.enadv.hifix;
        }
        en = rn1(enrnd, enfix);
        /* energy gain "modifier" */
        switch (Role_switch) {
        case PM_CLERIC:
        case PM_WIZARD:
            en *= 2;
            break;
        case PM_HEALER:
        case PM_KNIGHT:
            en = ((3 * en) / 2);
            break;
        case PM_BARBARIAN:
        case PM_VALKYRIE:
            en = ((3 * en) / 4);
        }
    }
    if (en <= 0)
        en = 1;
    if (u.ulevel < MAXULEV)
        u.ueninc[u.ulevel] = (xchar) en;
    return en;
}

/* return # of exp points for mtmp after nk killed */
int
experience(register struct monst *mtmp, register int nk)
{
    register struct permonst *ptr = mtmp->data;
    int i, tmp, tmp2;

    tmp = 1 + mtmp->m_lev * mtmp->m_lev;

    /*  For higher ac values, give extra experience */
    if ((i = find_mac(mtmp)) < 3)
        tmp += (7 - i) * ((i < 0) ? 2 : 1);

    /*  For very fast monsters, give extra experience */
    if (ptr->mmove > NORMAL_SPEED)
        tmp += (ptr->mmove > (3 * NORMAL_SPEED / 2)) ? 5 : 3;

    /*  For each "special" attack type give extra experience */
    for (i = 0; i < NATTK; i++) {
        tmp2 = ptr->mattk[i].aatyp;
        if (tmp2 > AT_BUTT) {
            if (tmp2 == AT_WEAP)
                tmp += 5;
            else if (tmp2 == AT_MAGC)
                tmp += 10;
            else
                tmp += 3;
        }
    }

    /*  For each "special" damage type give extra experience */
    for (i = 0; i < NATTK; i++) {
        tmp2 = ptr->mattk[i].adtyp;
        if (tmp2 > AD_PHYS && tmp2 < AD_BLND)
            tmp += 2 * mtmp->m_lev;
        else if ((tmp2 == AD_DRLI) || (tmp2 == AD_STON) || (tmp2 == AD_SLIM))
            tmp += 50;
        else if (tmp2 != AD_PHYS)
            tmp += mtmp->m_lev;
        /* extra heavy damage bonus */
        if ((int) (ptr->mattk[i].damd * ptr->mattk[i].damn) > 23)
            tmp += mtmp->m_lev;
        if (tmp2 == AD_WRAP && ptr->mlet == S_EEL && !Amphibious)
            tmp += 1000;
    }

    /*  For certain "extra nasty" monsters, give even more */
    if (extra_nasty(ptr))
        tmp += (7 * mtmp->m_lev);

    /*  For higher level monsters, an additional bonus is given */
    if (mtmp->m_lev > 8)
        tmp += 50;

#ifdef MAIL_STRUCTURES
    /* Mail daemons put up no fight. */
    if (mtmp->data == &mons[PM_MAIL_DAEMON])
        tmp = 1;
#endif

    if (mtmp->mrevived || mtmp->mcloned) {
        /*
         *      Reduce experience awarded for repeated killings of
         *      "the same monster".  Kill count includes all of this
         *      monster's type which have been killed--including the
         *      current monster--regardless of how they were created.
         *        1.. 20        full experience
         *       21.. 40        xp / 2
         *       41.. 80        xp / 4
         *       81..120        xp / 8
         *      121..180        xp / 16
         *      181..240        xp / 32
         *      241..255+       xp / 64
         */
        for (i = 0, tmp2 = 20; nk > tmp2 && tmp > 1; ++i) {
            tmp = (tmp + 1) / 2;
            nk -= tmp2;
            if (i & 1)
                tmp2 += 20;
        }
    }

    return (tmp);
}

void
more_experienced(register int exper, register int rexp)
{
    long oldexp = u.uexp,
         oldrexp = u.urexp,
         newexp = oldexp + exper,
         rexpincr = 4 * exper + rexp,
         newrexp = oldrexp + rexpincr;

    /* cap experience and score on wraparound */
    if (newexp < 0 && exper > 0)
        newexp = LONG_MAX;
    if (newrexp < 0 && rexpincr > 0)
        newrexp = LONG_MAX;

    if (newexp != oldexp) {
        u.uexp = newexp;
        if (flags.showexp)
            g.context.botl = TRUE;
        /* even when experience points aren't being shown, experience level
           might be highlighted with a percentage highlight rule and that
           percentage depends upon experience points */
        if (!g.context.botl && exp_percent_changing())
            g.context.botl = TRUE;
    }
    /* newrexp will always differ from oldrexp unless they're LONG_MAX */
    if (newrexp != oldrexp) {
        u.urexp = newrexp;
#ifdef SCORE_ON_BOTL
        if (flags.showscore)
            g.context.botl = TRUE;
#endif
    }
    if (u.urexp >= (Role_if(PM_WIZARD) ? 1000 : 2000))
        flags.beginner = 0;
}

/* e.g., hit by drain life attack */
void
losexp(const char *drainer) /* cause of death, if drain should be fatal */
{
    register int num;

    /* override life-drain resistance when handling an explicit
       wizard mode request to reduce level; never fatal though */
    if (drainer && !strcmp(drainer, "#levelchange"))
        drainer = 0;
    else if (resists_drli(&g.youmonst) || item_catches_drain(&g.youmonst))
        return;

    if (u.ulevel > 1) {
        pline("%s level %d.", Goodbye(), u.ulevel--);
        /* remove intrinsic abilities */
        adjabil(u.ulevel + 1, u.ulevel);
    } else {
        if (drainer) {
            g.killer.format = KILLED_BY;
            if (g.killer.name != drainer)
                Strcpy(g.killer.name, drainer);
            done(DIED);
        }
        /* no drainer or lifesaved */
        u.uexp = 0;
    }
    num = (int) u.uhpinc[u.ulevel];
    u.uhpmax -= num;
    if (u.uhpmax < 1)
        u.uhpmax = 1;
    u.uhp -= num;
    if (u.uhp < 1)
        u.uhp = 1;
    else if (u.uhp > u.uhpmax)
        u.uhp = u.uhpmax;

    num = (int) u.ueninc[u.ulevel];
    u.uenmax -= num;
    if (u.uenmax < 0)
        u.uenmax = 0;
    u.uen -= num;
    if (u.uen < 0)
        u.uen = 0;
    else if (u.uen > u.uenmax)
        u.uen = u.uenmax;

    if (u.uexp > 0)
        u.uexp = newuexp(u.ulevel) - 1;

    if (Upolyd) {
        num = monhp_per_lvl(&g.youmonst);
        u.mhmax -= num;
        u.mh -= num;
        if (u.mh <= 0) {
            /* in case we die here */
            Strcpy(g.killer.name, "fragility");
            g.killer.format = KILLED_BY;
            rehumanize();
        }
    }

    g.context.botl = TRUE;
}

/*
 * Make experience gaining similar to AD&D(tm), whereby you can at most go
 * up by one level at a time, extra expr possibly helping you along.
 * After all, how much real experience does one get shooting a wand of death
 * at a dragon created with a wand of polymorph??
 */
void
newexplevel(void)
{
    if (u.ulevel < MAXULEV && u.uexp >= newuexp(u.ulevel))
        pluslvl(TRUE);
}

void
pluslvl(boolean incr) /* true iff via incremental experience growth */
{             /*        (false for potion of gain level)    */
    int hpinc, eninc;

    if (!incr)
        You_feel("more experienced.");

    /* increase hit points (when polymorphed, do monster form first
       in order to retain normal human/whatever increase for later) */
    if (Upolyd) {
        hpinc = monhp_per_lvl(&g.youmonst);
        u.mhmax += hpinc;
        u.mh += hpinc;
    }
    hpinc = newhp();
    u.uhpmax += hpinc;
    u.uhp += hpinc;

    /* increase spell power/energy points */
    eninc = newpw();
    u.uenmax += eninc;
    u.uen += eninc;

    /* increase level (unless already maxxed) */
    if (u.ulevel < MAXULEV) {
        int newrank, oldrank = xlev_to_rank(u.ulevel);

        /* increase experience points to reflect new level */
        if (incr) {
            long tmp = newuexp(u.ulevel + 1);

            if (u.uexp >= tmp)
                u.uexp = tmp - 1;
        } else {
            u.uexp = newuexp(u.ulevel);
        }
        ++u.ulevel;
        pline("Welcome %sto experience level %d.",
              (u.ulevelmax < u.ulevel) ? "" : "back ",
              u.ulevel);
        if (u.ulevelmax < u.ulevel)
            u.ulevelmax = u.ulevel;
        adjabil(u.ulevel - 1, u.ulevel); /* give new intrinsics */
        newrank = xlev_to_rank(u.ulevel);
        if (newrank > oldrank)
            record_achievement(achieve_rank(newrank));
    }
    g.context.botl = TRUE;
}

/* compute a random amount of experience points suitable for the hero's
   experience level:  base number of points needed to reach the current
   level plus a random portion of what it takes to get to the next level */
long
rndexp(boolean gaining) /* gaining XP via potion vs setting XP for polyself */
{
    long minexp, maxexp, diff, factor, result;

    minexp = (u.ulevel == 1) ? 0L : newuexp(u.ulevel - 1);
    maxexp = newuexp(u.ulevel);
    diff = maxexp - minexp, factor = 1L;
    /* make sure that `diff' is an argument which rn2() can handle */
    while (diff >= (long) LARGEST_INT)
        diff /= 2L, factor *= 2L;
    result = minexp + factor * (long) rn2((int) diff);
    /* 3.4.1:  if already at level 30, add to current experience
       points rather than to threshold needed to reach the current
       level; otherwise blessed potions of gain level can result
       in lowering the experience points instead of raising them */
    if (u.ulevel == MAXULEV && gaining) {
        result += (u.uexp - minexp);
        /* avoid wrapping (over 400 blessed potions needed for that...) */
        if (result < u.uexp)
            result = u.uexp;
    }
    return result;
}

/*exper.c*/
