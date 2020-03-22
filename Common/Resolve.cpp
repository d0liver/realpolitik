//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Clarified Artistic License.
// In addition, to meet copyright restrictions you must also own at 
// least one copy of any of the following board games: 
// Diplomacy, Deluxe Diplomacy, Colonial Diplomacy or Machiavelli.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Clarified Artistic License for more details.
//
// You should have received a copy of the Clarified Artistic License
// along with this program; if not, write to the Open Source Initiative
// at www.opensource.org
//---------------------------------------------------------------------------------

/* 
 * resolve.c
 *
 * routines to handle resolution of orders
 *
 */

#include "MapData.h"
#include "Unit.h"
#include "Order.h"
#include "Game.h"
#include "Graph.h"
#include "Fails.h"
#include "OrdersDisplay.h"
#include "Resolve.h"
#include <string.h>

/* local prototypes */
void remselfsupp (GraphNode start, GraphNode finish);
void remselfsupp2(GraphNode start, GraphNode finish);
void cutsupports();
void reslv_moves(Orderlist orders[], bool ign_dislodge);
void civildisband (short country, short build, Orderlist orders);
void reslv_builds(Orderlist builds[]);
void reslv_retreats(Orderlist orders[]);
int follow (GraphNode sector);
int stronger (GraphNode reg1, GraphNode reg2);
int stronger_noselfsupp (GraphNode reg1, GraphNode reg2);
int strongest (GraphNode reg1);
int convbrkn (GraphNode support);
int broken (GraphNode support); 
int dislodged (GraphNode support);
int convdslg (GraphNode support);
int incomingdslg (GraphNode support);
void gendislodges();
int setdislodge();
bool unnecessaryconvoy( GraphNode fleet );
int strength_noselfsupp(GraphNode reg);

#define supp_against(support, attacker) \
     (gspec(support) == attacker)

void resolve_orders(Orderlist orders[], bool ign_units)
{
	if (dislodges > 0)
		reslv_retreats(orders);
	else if (adjustments)
		reslv_builds(orders);
	else
		reslv_moves(orders, ign_units);
}

/* perform the disband or retreat, depending */
void
reslv_retreats(Orderlist orders[])
{
	 GraphNode gp;
     register Order op;
     register bool found;
     char text[256];

     makegraph(orders, 0);

     /* find out what happens */
     for (gp = startnode(); isnode(gp); nextnode(gp))
          if (gtype(gp) == O_MOVE) {
               if (!gfails(gp))
                    follow(gp);
               if (gresolution(gp) == R_BOUNCES)
                    gresolution(gp) = R_DISBAND;
          }

     /* copy the resolve information back to the orders */
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
          if (isorder(gorder(gp)))
          {
               if (gresolution(gp) == R_DISBAND)
                    setofails(gorder(gp), R_BOUNCES);
               else
                    setofails(gorder(gp), gresolution(gp));
	       		order2string(text, gorder(gp), false, false);
		   		SetOrderLine(text, oindex(gorder(gp)));
		   }
     }
     
     Dislodge* dp = dslglist.Start(); 
     while (dp != dslglist.End())
     {          
          /* don't worry about already disbanded stuff, in fact, remove it */
          if (dp->Retreats() == NULL)
          {
          		Dislodge* np = dslglist.Next( dp );
          		dslglist.Delete( dp->Sector() );
          		dislodges--;
          		dp = np;
          	   	continue;
          }  
          	   	
          found = false;
          for (op = startorder(orders[dp->Owner()]); isorder(op); nextorder(op)) 
          {
          		if (dp->Sector() == ostart(op))
          		{
          			if (otype(op) == O_NMR)
          			{
          				dp->SetType(DB_NRR);
          				ofails(op) = R_DISBAND;
	       				order2string(text, op, false, false);
		   				SetOrderLine(text, oindex(op));
          			}
          			else if (orderfails(op))
          			{
          				dp->SetType(DB_ILL);
               		}
         			found = true;
          			break;
          		}
          }
          if (!found)
          {
              dp->SetType(DB_NRR);
          }
          
          dp = dslglist.Next(dp);
	}

}

void 
reslv_builds(Orderlist orders[])
{
     register short i, reg, build;
     register Order op;
     GraphNode gp;
     char text[256];
     
     // need to make graph to catch duplicate orders and for civil disbands
	 makegraph(orders, false);

     for (i = 1; i <= NUM_COUN; i++) 
     {
          build = countries[i].builds;
          for (op = startorder(orders[i]); isorder(op); nextorder(op)) 
          {
               reg = ostart(op);
               if (otype(op) == O_BUILD && !orderfails(op)) 
               {
               		if (reg == 0)
               		{
               			otype(op) = O_WAIVE;
 	       				order2string(text, op, false, false);
		   				SetOrderLine(text, oindex(op));
		   			}
              		else
               		{
                   		if (build > 0) 
                         	build--;
                    	else
                         	setofails(op, R_ADJUST);
                    }
               }
               else if ((otype(op) == O_REMOVE || otype(op) == O_DEF_REM) 
               			&& !orderfails(op)) 
               {
               		if (reg == 0)
               			civildisband(i, build, orders[i]);
               		if (build < 0)
                   		 build++;
                    else
                         setofails(op, R_ADJUST);                   	
               }
               if (ofails(op))
               {
 	       			order2string(text, op, false, false);
		   			SetOrderLine(text, oindex(op));
		   	   }
              	
          }
          /* if not all disband orders given */
          if (build < 0) 
          {
               /* civil disorder -- disband stuff farthest away */
               civildisband(i, build, orders[i]);
          }
          		
     }

	// set status of orders from graph
	for (gp = startnode(); isnode(gp); nextnode(gp)) 
	{
	  	if (isorder(gorder(gp)) && !ofails(gorder(gp)))
	  	{
	       	setofails(gorder(gp), gresolution(gp));
	       	order2string(text, gorder(gp), false, false);
		   	SetOrderLine(text, oindex(gorder(gp)));
	   	}
	} 

}

/* disband units farthest from home */
void
civildisband (short country, short build, Orderlist orders)
{
     register short i, distance, currdist;
     register Unit up, farthest;
     Order op;

     for (i = -build; i; i--) 
     {
          farthest = NULL;
          distance = 0;
          for (up = startunit(units[country]); isunit(up); nextunit(up)) 
          {
          	   // if this one is already being removed, move on
          	   if (gtype(getnode(usector(up))) == O_REMOVE)
          	   	   continue;
               if (utype(up) == U_ARM) 
               {
                    currdist = armydistance(up, country);
                    if (currdist > distance) 
                    {
                         distance = currdist;
                         farthest = up;
                    }
               }
               else 
               {
                    currdist = fleetdistance(up,country);
                    if (currdist > distance
                        || (currdist == distance && (farthest == NULL || utype(farthest) == U_ARM))) 
                    {
                         distance = currdist;
                         farthest = up;
                    }
               }
          }
		
		  op = neworder();
		  ounit(op) = utype(farthest);
		  ostart(op) = usector(farthest);
		  oscoast(op) = ucoast(farthest);
		  ocountry(op) = country;
		  otype(op) = O_DEF_REM;
		  ofails(op) = R_SUCCESS;
		  otext(op) = (char *) new char[2];
		  strcpy(otext(op), "*");
		  gtype(getnode(ostart(op))) = O_REMOVE;
		  addorder(orders, op, false);
     }

}


void 
reslv_moves(Orderlist orders[], bool ign_dislodge)
{
	register GraphNode gp, move;
	char text[256];
     
	makegraph(orders, ign_dislodge);

	// cut all supports
    cutsupports();

    // resolve moves that might dislodge units that are supported by units of same type 
    for (gp = startnode(); isnode(gp); nextnode(gp))
    {
    	// if support order against unit of same type
     	if (gtype(gp) == O_SUPP && !gfails(gp) && gdest(gp) != gspec(gp) 
     		&& gowner(gp) == gowner(gspec(gp)))
     	{
		    for (move = startnode(); isnode(move); nextnode(move)) 
 		    {
			  	// if this is the move we're supporting 
			  	if (gtype(move) == O_MOVE && move == gdest(gp) &&
			  	  	gdest(move) == gspec(gp))
			  	{
			  		// succeeds, and is strongest move into that area 
			      	if (!gfails(move) && strongest(move)) 
			  		{
	 		    		(void) follow(move);
	 				}
	 		  	}
     	    }
     	}
	}

    // resolve moves 
    for (gp = startnode(); isnode(gp); nextnode(gp)) 
    {
		if (gtype(gp) == O_MOVE && !gresolved(gp)) 
	  	{
	       (void) follow(gp);
	  	}
    }
     
    /* resolve self-dislodges */
    for (gp = startnode(); isnode(gp); nextnode(gp)) 
    {
     	/* if this move would dislodge a unit of its country */
	  	if (gtype(gp) == O_MOVE && !gresolved(gp)
	  	  	&& strongest(gp) && stronger(gp, gdest(gp))
	  	  	&& gowner(gp) == gowner(gdest(gp))) 
	  	{
	  		/* don't do it */
     		setgresolution(gp, R_FAILS);
	  	}
    }

    // one final resolution pass, just to be sure
    for (gp = startnode(); isnode(gp); nextnode(gp)) 
    {
		if (gtype(gp) == O_MOVE && !gresolved(gp)) 
	  	{
			if (!follow(gp))
				setgresolution(gp, R_FAILS);	
	  	}
    }

    /* deal with dislodges */
    if (!ign_dislodge)
	  	dislodges = setdislodge();

    /* set status of orders */
    for (gp = startnode(); isnode(gp); nextnode(gp)) {
	if (isorder(gorder(gp)))
	{
		setofails(gorder(gp), gresolution(gp));
	    order2string(text, gorder(gp), false, false);
		SetOrderLine(text, oindex(gorder(gp)));
	}
     }

}


void 
cutsupports ()
{
     register GraphNode gp;

     /* cut supports from all moves except convoyed armies and for attacks against us */
     for (gp = startnode(); isnode(gp); nextnode(gp)) 
     {
	  	if (gtype(gp) == O_MOVE && !gfails(gp) && !convoyed(gp)
	      && gtype(gdest(gp)) == O_SUPP && !gfails(gdest(gp))
	      && gstrength(gp) > 0                /* Loeb9 */
	      && !supp_against(gdest(gp), gp)     /* Rule X. */
	      && gowner(gdest(gp)) != gowner(gp)  /* can't break own support */
	      ) {  
	       setgresolution(gdest(gp), R_CUT);
	       decgstrength(gdest(gdest(gp)));
	  	}
     }

     /* mark possible dislodges of convoying fleets */
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp)
	      && gtype(gdest(gp)) == O_CONV && !gfails(gdest(gp))
	      && stronger(gp, gdest(gp))) 
	       setgresolution(gdest(gp), R_MAYBE);
     }

     /* check the convoyed armies */
     clearmarks();
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp)
	      && convoyed(gp) && !canconvoy(gp, gp)) 
	       setgresolution(gp, R_MAYBE);
     }

     /* cut supports by convoyed armies that *will* get there */
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp) && convoyed(gp)
	      && gtype(gdest(gp)) == O_SUPP && !gfails(gdest(gp))
	      && !supp_against(gdest(gp), gp)  /* Rule X. */
	      /* not supporting convoying unit */
	      && /* ( */
	      (gtype(gdest(gdest(gp))) != O_CONV 
	      /* not supporting attack against convoying unit */
//		   || gtype(gspec(gdest(gp))) != O_CONV /* Rule XII.5 */
		   // convoying unit not necessary for successful convoy
		   || unnecessaryconvoy( gdest(gdest(gp)) )
		   )
		  /* || stronger(gp, gdest(gp))) */
		  ) 
	  {
	       setgresolution(gdest(gp), R_CUT);
	       decgstrength(gdest(gdest(gp)));
	  }
     }

     /* clear the "maybes" */
     for (gp = startnode(); isnode(gp); nextnode(gp)) 
	  if (gresolution(gp) == R_MAYBE)
	       setgresolution(gp, R_NONE);

     /* mark dislodges of convoying fleets */
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp)
	      && gtype(gdest(gp)) == O_CONV && !gfails(gdest(gp))
	      && follow(gp))
	       setgresolution(gdest(gp), R_DISLODGE);
     }

     /* check the convoyed armies */
     clearmarks();
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp)
	      && convoyed(gp) && !canconvoy(gp, gp)) 
	       setgresolution(gp, R_FAILS);
     }

     /* cut supports by all convoyed armies */
     for (gp = startnode(); isnode(gp); nextnode(gp)) 
     {
	  if (gtype(gp) == O_MOVE && !gfails(gp) && convoyed(gp)
	      && gtype(gdest(gp)) == O_SUPP && !gfails(gdest(gp)) 
	      && !supp_against(gdest(gp), gp)  /* Rule X.a */
		  && gowner(gdest(gp)) != gowner(gp)  /* can't break own support */
	      && /* ( */
	      (gtype(gdest(gdest(gp))) != O_CONV 
		   && gtype(gspec(gdest(gp))) != O_CONV) /* Rule XII.5 */
		  /* || stronger(gp, gdest(gp))) */
		  ) 
	  {
	       setgresolution(gdest(gp), R_CUT);
	       decgstrength(gdest(gdest(gp)));
	  }
     }
     

     /* check all dislodged supports against attacker (Rule X.b) */
     for (gp = startnode(); isnode(gp); nextnode(gp)) {
	  if (gtype(gp) == O_MOVE && !gfails(gp) 
	      && gtype(gdest(gp)) == O_SUPP && !gfails(gdest(gp))
		  && gowner(gdest(gp)) != gowner(gp)  /* can't break own support */	 
		  // only care about supports against us
	      && supp_against(gdest(gp), gp) 
		  // can't dislodge with self-supports
		  && stronger_noselfsupp(gp, gdest(gp))) 
	  {
	       setgresolution(gdest(gp), R_CUT);
	       decgstrength(gdest(gdest(gp)));
	  }
     }
     
}

/* returns 1 if support order involving convoy is broken */
int convbrkn (GraphNode support)
{
     register GraphNode attacker;

     /* make sure we're supporting convoy, convoyed attack, or attack
      * against convoy 
      */
     if (gtype(gdest(support)) != O_CONV 
	 && (gtype(gdest(support)) != O_MOVE 
	     || !convoyed(gdest(support)))
	 && (gtype(gdest(support)) != O_MOVE
	     || gtype(gdest(gdest(support))) != O_CONV))
	  return 0;

     for (attacker = startnode(); isnode(attacker); nextnode(attacker))
	  
	  /* if attacker is going into support sector */
	  if (!gfails(attacker)
	      && gtype(attacker) == O_MOVE && gdest(attacker) == support) {

	       /* if attacker is being convoyed */
	       if (convoyed(attacker)
	       /* and support is into a body of water with convoying fleet */
	       /* (Rule XII.5) */
	      && (gtype(gdest(support)) == O_CONV
		  || gtype(gspec(support)) == O_CONV))
	       /* forget it */
		   continue;

	       /* otherwise support fails */
	       setgresolution(support, R_CUT);

	       /* decrement strength of supported sector */
	       decgstrength(gdest(support));
	       return 1;
	  }
     return 0;
}

/* returns 1 if any support order is broken */
int broken (GraphNode support) 
{
     register GraphNode attacker;

     for (attacker = startnode(); isnode(attacker); nextnode(attacker))

	  /* if attacker is going into support sector */
	  if (!gfails(attacker) 
	      && gtype(attacker) == O_MOVE && gdest(attacker) == support
          /* and support is not against the attacker */
	      && gspec(support) != attacker) 
	 {
	       /* if attacker is being convoyed */
	       if (convoyed(attacker)
	       /* and support is into a body of water with convoying fleet */
	       /* (Rule XII.5) */
	      && (gtype(gdest(support)) == O_CONV
		  || gtype(gspec(support)) == O_CONV))
	       /* forget it */
		   continue;

	       /* otherwise, support fails */
	       setgresolution(support, R_CUT);

	       /* decrement strength of supported sector */
	       decgstrength(gdest(support));
	       return 1;
	  }
	  return 0;
}

/* returns 1 if support order is dislodged */
int dislodged (GraphNode support) 
{
     GraphNode attacker;

     if (gfails(support))
	  return 0;

     /* if sector we're supporting isn't moving, this doesn't apply */
     if (gtype(gdest(support)) != O_MOVE)
	  return 0;

     attacker = gspec(support);
     if (gfails(attacker))
	  return 0;

     /* if sector we're supporting into is attacking us and is stronger */
     if (gdest(attacker) == support && stronger(attacker, support)
     /* and isn't our own unit and isn't convoyed */
	 && gowner(attacker) != gowner(support) && !convoyed(attacker)) {

	  /* unit is dislodged */
	  setgresolution(support, R_DISLODGE);
	  decgstrength(gdest(support));
	  return 1;
     }
     return 0;
}

/* returns 1 if support is dislodged by convoy that can definitely make it */
int convdslg (GraphNode support)
{

     if (gfails(support))
	  return 0;

     else
	  return 1;
}

/* returns 1 if support order against convoy is dislodged by convoy */
/* not used, right now -- my interpretation is that this isn't possible */
int incomingdslg (GraphNode support)
{
     GraphNode attacker;

     if (gfails(support))
	  return 0;

     /* if sector we're supporting isn't moving, this doesn't apply */
     if (gtype(gdest(support)) != O_MOVE 
	 && gtype(gdest(gdest(support))) != O_CONV)
	  return 0;

     attacker = gdest(gspec(support));
     if (gfails(attacker))
	  return 0;

     /* if sector we're supporting into is attacking us and is stronger */
     if (gdest(attacker) == support && stronger(attacker, support)
     /* and isn't our own unit */
	 && gowner(attacker) != gowner(support)) {
	      
	  /* unit is dislodged */
	  setgresolution(support, R_DISLODGE);
	  decgstrength(gdest(support));
	  return 1;
     }
     return 0;
}

/* returns 1 if order generates a move from that sector */
int follow (GraphNode sector)
{
     /* if not a moving order */
     if (gtype(sector) != O_MOVE)     
	  return 0; 

     /* if already resolved */
     if (gresolved(sector))             
	  return (!(gfails(sector))); 

     /* if we've come full circle */
     if (gmarked(sector)) 
     {     
	  	unsetgmark(sector);
	  	/* can't switch with neighbor */
	  	/* unless convoyed! */
	  	if (gdest(gdest(sector)) == sector 
	      	&& !convoyed(sector) && !convoyed(gdest(sector)))  
	       	return 0;
	  	else 
	       	return 1;
     }
     
     /* if dest coming at us full blast */
     if (gdest(gdest(sector)) == sector && stronger(gdest(sector), sector)
     /* and neither of us are convoyed */
         && !convoyed(sector) && !convoyed(gdest(sector))) 
     {
	  	return 0;
     }

     /* if strongest vying for dest */
     if (!strongest(sector))
	  	return 0;

     /* if no unit there */
     if (gtype(gdest(sector)) == O_UNKN) 
     {       
	  	setgresolution(sector, R_SUCCESS);
	  	decgbounce(sector);
	  	incgbounce(gdest(sector));
	  	return 1;
     }

     /* see if unit successfully moves */
     setgmark(sector, 1);
     if (follow(gdest(sector))) 
     {          
	  	/* follow could lead to a move that dislodges this one */
	  	if (gfails(sector))
	       	return 0; 
	    /* way is clear, go for it */
	  	setgresolution(sector, R_SUCCESS);
	  	decgbounce(sector);
	  	incgbounce(gdest(sector));
	  	unsetgmark(sector);
	  	return 1;
     }
     unsetgmark(sector);

     /* remove supports by us against us */
     /*** does this need to be here, or could it be combined with 
          remselfsupp2 above? (If it ain't broke, don't fix it) */
     remselfsupp(sector, gdest(sector));

     /* if our strength is greater than that of our destination */
     if (strongest(sector) && stronger(sector, gdest(sector))) 
     {
     	/* and our destination is not our own unit */
		if (gowner(sector) != gowner(gdest(sector))) 
	 	{
	  		/* then we plow our way through! */
	  		setgresolution(sector, R_SUCCESS);
	  		setgresolution(gdest(sector), R_DISLODGE);
	  		decgbounce(sector);
	  		incgbounce(gdest(sector));
	  		return 1;
     	}
     	/* no self-dislodges (mark failure later) */
     	else
     	{
	  		incgbounce(gdest(sector));
     		return 0;
     	}
     }
     
     /* anything else is just failure */
     if (!gresolved(sector))
     	setgresolution(sector, R_FAILS);
     	
     return 0;
}

/* returns true if reg1 unit can overpower reg2 unit */
int stronger (GraphNode reg1, GraphNode reg2)
{
     if (gtype(reg1) != O_MOVE)
	  return 0;
	  
     if (gtype(reg2) == O_MOVE)
	  if (gdest(reg1) == gdest(reg2) 
	      || (gdest(reg2) == reg1 && gdest(reg1) == reg2))
	       return (gstrength(reg1) > gstrength(reg2));
	  else
	       return (gstrength(reg1) > 1);
     else
	  return (gstrength(reg1) > gstrength(reg2));
}

/* returns true if reg1 unit can overpower reg2 unit w/o support against own unit */
int stronger_noselfsupp(GraphNode reg1, GraphNode reg2)
{
     if (gtype(reg1) != O_MOVE)
	  return 0;

	if (gtype(reg2) == O_MOVE)
	{
		if (gdest(reg1) == gdest(reg2) 
	      || (gdest(reg2) == reg1 && gdest(reg1) == reg2))
	       return (strength_noselfsupp(reg1) > gstrength(reg2));
		else
	       return (strength_noselfsupp(reg1) > 1);
	}
	else
	{
		return (strength_noselfsupp(reg1) > gstrength(reg2));
	}
}

//#define PATCH

/* returns true if we're the strongest going into destination */
int strongest (GraphNode reg1) 
{
	register GraphNode dest, other;

	dest = gdest(reg1);
	for (other = startnode(); isnode(other); nextnode(other)) 
	{
		/* check all moves into our destination */
	  	if (gtype(other) == O_MOVE && gdest(other) == dest 
	  		// not this move
			&& other != reg1
	      	// that haven't failed or have bounced here 
	      	&& ( !gfails(other) || gresolution(other) == R_BOUNCES || gresolution(other) == R_DISLODGE) )
		{
			/* cf. rule IX.7 */
			/* other guy dislodged by attack from destination */
			if (gdest(dest) == other
				&& stronger(dest, other)
				&& follow(dest))
			{
				setgresolution(other, R_DISLODGE);
			}

			/* we're dislodged by attack from destination */
			else if (gdest(dest) == reg1 && stronger(dest, reg1))
			{
// only mark as dislodged by dislodging unit   
//			 	setgresolution(reg1, R_DISLODGE);
				return 0;
			}

		   // if we're stronger 
		   else if (stronger(reg1, other)) 
		   {
// if we fail for some other reason, other guy *might* succeed    
//				setgresolution(other, R_FAILS); 
		   }
	       // if other guy is stronger
	       else if (stronger(other, reg1))  
	       {   
		    	setgresolution(reg1, R_FAILS);
		   }
	       else // same strength
	       {      
				if (gresolution(other) == R_DISLODGE)
					setgresolution(other, R_BNCE_DIS);
 				else
 					setgresolution(other, R_BOUNCES);
		 		setgresolution(reg1, R_BOUNCES);
		 		incgbounce(dest);
		   }
		}
	}

	if (!gfails(reg1))
		return 1;
	else
		return 0;
}

/* look for supports against our own unit at finish and remove */
/* C.f. Rule IX.3 */
void remselfsupp (GraphNode start, GraphNode finish)
{
     register GraphNode support;

     for (support = startnode(); isnode(support); nextnode(support)) 
     {
	  /* if the move is a support of the attack, */
	  if (gtype(support) == O_SUPP && gdest(support) == start
	  /* succeeds, and the destination has the same owner as the support */
	      && !gfails(support) && gowner(support) == gowner(finish)
	      /* and this move is the strongest going in */
	      && strongest(start)
	      ) 
	  {
	       /* remove support */
	       setgresolution(support, R_FAILS);
	       decgstrength(start);
	  }
     }

}

/* look for supports by us against our unit moving into finish and remove */
/* C.f. Rule IX.3 */
void remselfsupp2(GraphNode start, GraphNode finish)
{
     register GraphNode support;

     for (support = startnode(); isnode(support); nextnode(support)) 
     {
	  /* if the move is a support of an attack against our move */
	  if (gtype(support) == O_SUPP && !gfails(support)
	  			&& gdest(support) != start 
	  			&& gspec(support) == finish && gspec(support) != gdest(support)
	  /* and has the same owner as our move */
	  			&& gowner(support) == gowner(start))
	  {
	       /* remove support */
	       setgresolution(support, R_FAILS);
	       decgstrength(gdest(support));
	  }
     }

}


int setdislodge()
{
     GraphNode gp;
     Sector destp;
     Order op;
     bool order_found = false;
     Neighbor lp, retreats = NULL;
     Unit up;
     short dcount = 0;
     DislodgeType dtype = DB_GEN;

	 dslglist.Clean();

     for (gp = startnode(); isnode(gp); nextnode(gp)) 
     {
	  	/* look for moves that dislodge */
	  	if (!gfails(gp) && gtype(gp) == O_MOVE
	      && (gresolution(gdest(gp)) == R_DISLODGE || 
	      	  gresolution(gdest(gp)) == R_NSO_DIS || 
	      	  gresolution(gdest(gp)) == R_CUT_DIS))
	    {
	       /* copy information */
	       up = getunit(units[gowner(gdest(gp))], U_NON, gindex(gdest(gp)), C_NONE);
	       if (up == NULL)
	       		continue;
	       destp = gsector(gdest(gp));
	       /* get neighbors */
	       if (utype(up) == U_ARM) 
		    	lp = startneighbor(landneigh(destp));
	       else if (!isbicoastal(destp) || ucoast(up) == scoast1(destp))
		    	lp = startneighbor(coast1neigh(destp));
	       else
		    	lp = startneighbor(coast2neigh(destp));
	       /* check retreats */
	       retreats = NULL;
	       while (isneighbor(lp)) 
	       {
		    	/* if the neighboring sector is not the attacking sector */
		    	if (getsector(map, nsector(lp)) != gsector(gp) 
		    	/* and there hasn't been a bounce there */
					&& !(gbounce(getnode(nsector(lp))))
					&& !(season == S_FALL && isice(getsector(map, nsector(lp)))))
			 		addneighbor(&retreats, nsector(lp), ncoast(lp));
		    	nextneighbor(lp);
	       }
	       
	       dtype = DB_GEN;
	       /* if no retreats, disband */
	       if (retreats == NULL) 
		    	setgresolution(gdest(gp), R_DISBAND);
		    	
		   /* else if no orders, civil disobediance */
           else
           {
           	    op = orders[gowner(gdest(gp))];
           	    order_found = false;
      			while (op-orders[gowner(gdest(gp))] < MAX_ORD && isorder(op)) 
      			{
     				if (otype(op) != O_NONE && otype(op) != O_NMR &&
     					otype(op) != O_NMR_DIS)
     				{
     					order_found = true;
     					break;
     				}
     				nextorder(op);
     			}
          		if (gStrictNMR && !order_found)
          		{
                	dtype = DB_NMR;
                	setgresolution(gdest(gp), R_DISBAND);
                	if (retreats != NULL)
                		cleanneighbors(&retreats);
     			}
           }
		    
		   dslglist.Add(utype(up), usector(up), ucoast(up), gowner(gdest(gp)), 
		   		dtype, retreats);
		   dcount++;
	  	}
     }

     return dcount;
}

void gendislodges()
{
	makegraph(orders, false);
	dislodges = setdislodge();
}

//----------------------------------------------------------------------------
// @ unnecessaryconvoy()
//----------------------------------------------------------------------------
// returns true if convoying fleet is unnecessary to complete convoy
//----------------------------------------------------------------------------
bool 
unnecessaryconvoy( GraphNode fleet )
{
	if (gtype(fleet) != O_CONV)
		return false;
	
	// mark this position as already visited
	clearmarks();
	setgmark(fleet, 1);
	
	bool result = (canconvoy( gdest(fleet), gdest(fleet) ) != 0);
	clearmarks();

	return result;
}

//----------------------------------------------------------------------------
// @ strength_noselfsupp()
//----------------------------------------------------------------------------
// returns strength without self-supports against our own destination
//----------------------------------------------------------------------------
int
strength_noselfsupp(GraphNode reg)
{
	int strength = gstrength(reg);	
	GraphNode gp;
	for (gp = startnode(); isnode(gp); nextnode(gp))
	{
		if (!gfails(gp) 
			&& gtype(gp) == O_SUPP
			&& gdest(gp) == reg
			&& gowner(gp) == gowner(gdest(reg))
			)
		{
			--strength;
		}
	}

	return strength;
}