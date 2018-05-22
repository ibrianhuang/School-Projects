#include <stdlib.h>
#include <stdbool.h>

// ----- Structs -----

typedef struct queryGroup queryGroup;

struct queryGroup
{
    int group[4]; //Contains a 4-element group of indices
    int weight; //The # of majority elements this group is representing
    int majorityIndex1, majorityIndex2; //The indices of the group array that contain majority indices
    queryGroup *nextGroup, *pastGroup; //Link List compatibility
};

typedef struct
{
    queryGroup *start, *end;
    int length;
} linkListEnds;

// ----- General Functions -----

void replace4Values( int group[4], int v1, int v2, int v3, int v4)
{
    group[0] = v1;
    group[1] = v2;
    group[2] = v3;
    group[3] = v4;
}

void free3groups( queryGroup *group1, queryGroup *group2, queryGroup *group3)
{
    free( group1 );
    free( group2 );
    free( group3 );
}

// ----- Link List Functions -----

void addToLinkList( linkListEnds* list, queryGroup* newEntry )
{
    if( list->length == 0 )
    {
        list->start = newEntry;
        newEntry->pastGroup = NULL;
    }
    else
    {
        list->end->nextGroup = newEntry;
        newEntry->pastGroup = list->end;
    }
    list->end = newEntry;
    newEntry->nextGroup = NULL;
    list->length++;
}

void newFront( linkListEnds* list, queryGroup* newEntry)
{
    if( list->length == 0)
        list->end = newEntry;
    else
        list->start->pastGroup = newEntry;
    newEntry->nextGroup = list->start;
    list->start = newEntry;
    newEntry->pastGroup = NULL;
    list->length++;
}

void placeBasedOffWeight( linkListEnds* list, queryGroup* newEntry)
{
    if( list->length == 0 )
    {
        addToLinkList( list, newEntry );
        return;
    }

    queryGroup *current;
    for( current = list->end; current != NULL; current = current->pastGroup)
        if( current->weight <= newEntry->weight )
        {
            newEntry->nextGroup = current->nextGroup;
            current->nextGroup = newEntry;
            newEntry->pastGroup = current;
            if( list->end == current )
                list->end = newEntry;
            else
                newEntry->nextGroup->pastGroup = newEntry;
            list->length++;
            return;
        }

    newFront( list, newEntry);
    return;
}

queryGroup* popFront( linkListEnds* list )
{
    if (list->length == 0)
        return NULL;

    queryGroup *formerFront = list->start;
    list->start = list->start->nextGroup;
    if (list->length > 1)
    {
        list->start->pastGroup = NULL;
        formerFront->nextGroup = NULL;
    }
    else
        list->end = NULL;
    list->length--;
    return formerFront;
}

void cloneAndPlace( linkListEnds *list, queryGroup *entry)
{
    queryGroup *newEntry = malloc( sizeof(queryGroup) );

    replace4Values( newEntry->group, entry->group[0], entry->group[1], entry->group[2], entry->group[3]);
    newEntry->weight = entry->weight;
    newEntry->majorityIndex1 = entry->majorityIndex1;
    newEntry->majorityIndex2 = entry->majorityIndex2;
    newEntry->nextGroup = newEntry->pastGroup = NULL;
    placeBasedOffWeight( list, newEntry );
}

// ----- Tournament Functions -----

// Places all the groups into lists depending on whether the majority location are known (4s) or if they are unknown (2s); 0s are thrown out
// Returns 0 on normal; -1 on error
int setup( int n, linkListEnds* knownGroups, linkListEnds* unknownGroups, linkListEnds* firstOf, queryGroup* oddGroup )
{
    int iterator, queryResult;
    bool first0, first2, first4;
    first0 = first2 = first4 = false;

    knownGroups->length = unknownGroups->length = firstOf->length = 0;
    knownGroups->start = knownGroups->end = unknownGroups->start = unknownGroups->end = firstOf->start = firstOf->end = NULL;
    for( iterator = 0; iterator < n/4; ++iterator)
    {
        queryGroup *newGroup = malloc( sizeof(queryGroup) );
        replace4Values( newGroup->group, iterator*4+1, iterator*4+2, iterator*4+3, iterator*4+4);
        queryResult = QCOUNT( 1, newGroup->group);
        if( queryResult == 0 )
        {
            //printf("0\n");
            if ( !first0 )
            {
                newGroup->majorityIndex1 = newGroup->majorityIndex2 = -1;
                newGroup->weight = 0;
                first0 = true;
                cloneAndPlace( firstOf, newGroup);
            }
            free( newGroup );
        }
        else if ( queryResult == 2 )
        {
            newGroup->majorityIndex1 = newGroup->majorityIndex2 = -1;
            newGroup->weight = 2;
            if ( !first2 )
            {
                first2 = true;
                cloneAndPlace( firstOf, newGroup);
            }
            placeBasedOffWeight( unknownGroups, newGroup);
        }
        else if ( queryResult == 4 )
        {
            if ( !first4 )
            {
                first4 = true;
                cloneAndPlace( firstOf, newGroup);
            }
            newGroup->majorityIndex1 = 0;
            newGroup->majorityIndex2 = 1;
            newGroup->weight = 4;
            placeBasedOffWeight( knownGroups, newGroup);
        }
        else // == -1; Error
            return -1;
    }

    replace4Values( oddGroup->group, n, n-1, n-2, n-3 ); //indices after weight are filler
    oddGroup->weight = n % 4; //Can be zero if n is divisible by 4
    oddGroup->nextGroup =  oddGroup->pastGroup = NULL;
    oddGroup->majorityIndex1 = oddGroup->majorityIndex2 = -1;
    return 0;
}

//Find the majority of both groups
//Returns 0 if both already found; 1 if same majority; 2 if different majority; 3 if majority found, but relation between groups unknown
//Returns -1 on error
int findMajority( queryGroup *leftGroup, queryGroup *rightGroup )
{
    if( rightGroup->majorityIndex1 != -1 ) //Assumes if rightGroup has majority, left Group has to have it as well
        return 0; //Majority is already found

    int compareArray[4];
    int result1, result2;

    if( leftGroup->majorityIndex1 == -1 ) //Both unknown
    {
        replace4Values( compareArray, leftGroup->group[0], leftGroup->group[1], rightGroup->group[0], rightGroup->group[1] );
        result1 = QCOUNT(1, compareArray);
        if( result1 == 4 )
        {
            leftGroup->majorityIndex1 = rightGroup->majorityIndex1 = 0;
            leftGroup->majorityIndex2 = rightGroup->majorityIndex2 = 1;
            return 1;
        }
        else if ( result1 == -1 ) //Error
            return -1;

        compareArray[0] = leftGroup->group[2];
        result2 = QCOUNT(1, compareArray);
        if( result2 == 4 )
        {
            leftGroup->majorityIndex1 = 2;
            rightGroup->majorityIndex1 = 0;
            leftGroup->majorityIndex2 = rightGroup->majorityIndex2 = 1;
            return 1;
        }
        else if ( result2 == -1 ) //Error
            return -1;
        else if ( result1 == result2 )
        {
            leftGroup->majorityIndex1 = 0;
            leftGroup->majorityIndex2 = 2;
        }
        else //result1 != result2
        {
            leftGroup->majorityIndex1 = 1;
            leftGroup->majorityIndex2 = 3;
        }
    }
    //If here, only right is unknown
    replace4Values( compareArray, leftGroup->group[ leftGroup->majorityIndex1 ], leftGroup->group[ leftGroup->majorityIndex2 ],
                    rightGroup->group[2], rightGroup->group[3] ); //Use 2 and 3 for probability reasons
    rightGroup->majorityIndex1 = 2;
    rightGroup->majorityIndex2 = 3;
    result1 = QCOUNT( 1, compareArray);
    if ( result1 == 4 )
        return 1;
    else if (result1 == 0)
        return 2;
    else if ( result1 == -1 ) //Error
        return -1;
    else //result1 == 2
    {
        rightGroup->majorityIndex1 = 0;
        rightGroup->majorityIndex2 = 1;
        return 3;
    }
}

// Converts an unknown Group to a known group
// Returns -1 on error; 0 normally
int filterOutUnknowns( linkListEnds *knownGroups, linkListEnds *unknownGroups )
{
    if( (unknownGroups->length == 0) || (unknownGroups->length == 1 && knownGroups->length == 0) )
    {
        printf("Invalid input given to filterOutUnknowns\n");
        return -1;
    }

    // Can assume that unknownGroups has a weight >= 1 and that a second group exists in either known or unknown
    queryGroup *leftGroup = ( knownGroups->length > 0 ? popFront( knownGroups ) : popFront( unknownGroups ) ); //Left Has to be known if there is only 1 known
    queryGroup *rightGroup = popFront( unknownGroups );
    if( leftGroup == NULL || rightGroup == NULL )
    {
        printf("Tried to remove from an empty list\n");
        return -1;
    }
    int results = findMajority( leftGroup, rightGroup );

    switch (results)
    {
        case -1: //Error
            return -1;
        case 0: //Shouldn't happen
            return -1;
        case 1: //Same majority
        {
            leftGroup->weight = leftGroup->weight + rightGroup->weight;
            placeBasedOffWeight( knownGroups, leftGroup);
            free( rightGroup );
            break;
        }
        case 2: //Different Majority
        {
            int differenceBetweenGroups = leftGroup->weight - rightGroup->weight;
            if ( differenceBetweenGroups > 0 )
            {
                leftGroup->weight = differenceBetweenGroups;
                placeBasedOffWeight( knownGroups, leftGroup);
                free(rightGroup);
            }
            else if ( differenceBetweenGroups < 0 )
            {
                rightGroup->weight = differenceBetweenGroups * -1;
                placeBasedOffWeight( knownGroups, rightGroup);
                free(leftGroup);
            }
            else // differenceBetweenGroups == 0
            {
                free(rightGroup);
                if( knownGroups->length == 0 && unknownGroups->length >= 1 ) //Don't want to do 2 comparisons for 2 unknowns
                {
                    leftGroup->weight = 0;
                    placeBasedOffWeight(knownGroups, leftGroup);
                }
                else
                    free(leftGroup);
            }
            break;
        }
        case 3: //Unknown relation
        {
            placeBasedOffWeight( knownGroups, leftGroup);
            placeBasedOffWeight( knownGroups, rightGroup);
            break;
        }
    }
    return 0;
}

// Compares 4 knowns if length >= 4, else compares 2 knowns
// Returns -1 on error; 0 normally
int compareKnowns( linkListEnds *knownGroups)
{
    int compareArray[4];
    int result1, result2, result3, differenceBetweenGroups;
    queryGroup *leftGroup, *rightGroup, *group1, *group2, *group3, *group4, *winner;

    //Goal in this code is to find the winner
    if( knownGroups->length <= 1) //Done comparing
        return 0;
    else if( knownGroups->length < 4) //Compare 2 groups
    {
        leftGroup = popFront( knownGroups );
        rightGroup = popFront( knownGroups );
        replace4Values( compareArray, leftGroup->group[leftGroup->majorityIndex1], leftGroup->group[leftGroup->majorityIndex2],
                        rightGroup->group[rightGroup->majorityIndex1], rightGroup->group[rightGroup->majorityIndex2]);
        result1 = QCOUNT( 1, compareArray); //Comparing majority indices of both groups

        switch (result1)
        {
            case -1: //Error
                return -1;
            case 2: //Shouldn't happen, as placed 2 same from left and 2 same from right
                return -1;
            case 4: //Same majority
            {
                leftGroup->weight = leftGroup->weight + rightGroup->weight;
                winner = leftGroup;
                free( rightGroup );
                break;
            }
            case 0: //Different Majority
            {
                int differenceBetweenGroups = leftGroup->weight - rightGroup->weight;
                if ( differenceBetweenGroups > 0 )
                {
                    leftGroup->weight = differenceBetweenGroups;
                    winner = leftGroup;
                    free(rightGroup);
                }
                else //if ( differenceBetweenGroups <= 0 )
                {
                    rightGroup->weight = differenceBetweenGroups * -1; //Negative doesn't matter if 0
                    winner = rightGroup;
                    free(leftGroup);
                }
                break;
            }
        }
    }
    else // knownGroups->length >= 4
    {
        group1 = popFront( knownGroups );
        group2 = popFront( knownGroups );
        group3 = popFront( knownGroups );
        group4 = popFront( knownGroups );
        replace4Values(compareArray, group1->group[group1->majorityIndex1], group2->group[group2->majorityIndex1],
                       group3->group[group3->majorityIndex1], group4->group[group4->majorityIndex1]);

        result1 = QCOUNT( 1, compareArray);

        //polarity refers to the majority of a group or the value the weight represents
        //Remember that knownGroups is sorted by weight; Fact is used below to make a lot of assumptions
        if(result1 == 4 ) //All same polarity
        {
            group1->weight = group1->weight + group2->weight + group3->weight + group4->weight;
            winner = group1;
            free3groups( group2, group3, group4);
        }
        else if (result1 == -1) //Error
            return -1;
        else if (  group1->weight == group4->weight ) //All same weight
        {
            if( result1 == 0) //Cancel each other out
                return 0;
            else //result1 == 2
            {
                compareArray[2] = group1->group[group1->majorityIndex2]; //Replacing second and third index with second majority of group 1 and 2
                compareArray[3] = group2->group[group2->majorityIndex2];
                result2 = QCOUNT(1, compareArray);

                switch (result2)
                {
                    case -1: //Error
                        return -1;
                    case 2: //Shouldn't happen, as placed 2 same from left and 2 same from right
                        return -1;
                    case 0: //Groups 1 and 2 cancel each other; Groups 3 and 4 have same polarity
                    {
                        group3->weight = group3->weight + group4->weight;
                        winner = group3;
                        free3groups( group1, group2, group4);
                        break;
                    }
                    case 4: //Groups 3 and 4 cancel each other out; Groups 1 and 2 have same polarity
                    {
                        group1->weight = group1->weight + group2->weight;
                        winner = group1;
                        free3groups( group2, group3, group4);
                        break;
                    }
                }
            }
        }
        else if ((group1->weight == group3->weight) || (group2->weight == group4->weight)) // 3 are the same weight
        {
            queryGroup *differentWeight, *majorityWeight; //differentWeight is group that isn't the same as the other 3; majorityWeight represents the 3 groups with the same weight
            if (group1->weight == group3->weight) //Find where odd group is
            {
                differentWeight = group4;
                majorityWeight = group1;
            }
            else //group2->weight == group4->weight
            {
                differentWeight = group1;
                majorityWeight = group4;
            }

            if (result1 == 0) //2 0-majorities, 2 1-majorities
            {
                differenceBetweenGroups = differentWeight->weight - majorityWeight->weight; // 2 groups of the same weight will cancel each other out
                if ( differenceBetweenGroups > 0 )
                {
                    differentWeight->weight = differenceBetweenGroups; // differentWeight is greater, so that group moves on
                    winner = differentWeight;
                    free3groups( group2, group3, majorityWeight);
                }
                else //if ( differenceBetweenGroups <= 0 )
                {
                    replace4Values(compareArray, group2->group[group2->majorityIndex1], group2->group[group2->majorityIndex2],
                                   group3->group[group3->majorityIndex1], group3->group[group3->majorityIndex2]);
                    result2 = QCOUNT(1, compareArray);

                    switch( result2 ) // Finding out which of the same majority is the opposite polarity of differentWeight
                    {
                        case -1: // Error
                            return -1;
                        case 2: // Not possible, comparing 2 of the same vs 2 of the same
                            return -1;
                        case 4: // Groups 2 and 3 are of the same polarity
                        {
                            group2->weight = differenceBetweenGroups * -1;
                            winner = group2;
                            free3groups( group3, differentWeight, majorityWeight );
                            break;
                        }
                        case 0: // Groups 2 and 3 are of opposite polarity
                        {
                            majorityWeight->weight = differenceBetweenGroups * -1;
                            winner = majorityWeight;
                            free3groups( group2, group3, differentWeight );
                            break;
                        }
                    }
                }
            }
            else //result1 == 2; 1 of one polarity, 3 of the other polarity
            {
                replace4Values(compareArray, group2->group[group2->majorityIndex1], group3->group[group3->majorityIndex1],
                                differentWeight->group[differentWeight->majorityIndex1], differentWeight->group[differentWeight->majorityIndex2]);
                result2 = QCOUNT(1, compareArray);

                switch( result2 )
                {
                    case -1: //Error
                        return -1;
                    case 4: // groups 2, 3 and differentWeight are of the same polarity
                    {
                        differentWeight->weight = differentWeight->weight + group2->weight; //groups 3 and majorityWeight cancel each other out
                        winner = differentWeight;
                        free3groups( group2, group3, majorityWeight);
                        break;
                    }
                    case 2: // groups 2 and 3 are of different polarities so they cancel out
                    {
                        differentWeight->weight = differentWeight->weight + majorityWeight->weight; // remaining majority and differentWeight are of same polarity
                        winner = differentWeight;
                        free3groups( group2, group3, majorityWeight);
                        break;
                    }
                    case 0: // groups 2 and 3 are of the same polarity, but different from differentWeight
                    {
                        differenceBetweenGroups = differentWeight->weight - (3*majorityWeight->weight); // differentWeight is the odd one out
                        if ( differenceBetweenGroups > 0 )
                        {
                            differentWeight->weight = differenceBetweenGroups; // differentWeight is greater, so the weight of differentWeight minus the weights of the majority moves on
                            winner = differentWeight;
                            free3groups( group2, group3, majorityWeight);
                        }
                        else //if ( differenceBetweenGroups <= 0 )
                        {
                            majorityWeight->weight = differenceBetweenGroups * -1; // majorityWeight is greater, so the weights of the majority minus the differentWeight are combined
                            winner = majorityWeight;
                            free3groups( group2, group3, differentWeight);
                        }
                        break;
                    }
                }
            }
        }
        else if ( (group1->weight == group2->weight) || (group2->weight == group3->weight) || (group3->weight == group4->weight) ) // 2 groups with same weight
        {
            queryGroup *matching1, *matching2, *different1, *different2;
            if( group1->weight == group2->weight) //Find which 2 groups are of the same weight
            {
                matching1 = group1;
                matching2 = group2;
                different1 = group3;
                different2 = group4;
            }
            else if( group2->weight == group3->weight)
            {
                matching1 = group2;
                matching2 = group3;
                different1 = group1;
                different2 = group4;
            }
            else //if( group3->weight == group4->weight)
            {
                matching1 = group3;
                matching2 = group4;
                different1 = group1;
                different2 = group2;
            }

            if ( result1 == 0 )
            {
                replace4Values(compareArray, different1->group[different1->majorityIndex1], different1->group[different1->majorityIndex2],
                               different2->group[different2->majorityIndex1], different2->group[different2->majorityIndex2]);
                result2 = QCOUNT(1, compareArray);

                switch ( result2 )
                {
                    case -1: //Error
                        return -1;
                    case 2: // shouldn't happen
                        return -1;
                    case 4: // the different weights are the same polarity; the matching weights are of the same polarity
                    {
                        differenceBetweenGroups = different1->weight + different2->weight - (2 * matching1->weight);
                        if (differenceBetweenGroups > 0)
                        {
                            different1->weight = differenceBetweenGroups; // different weights are greater, so the weight of differentWeight minus the weights of the matching move on
                            winner = different1;
                            free3groups( matching1, matching2, different2);
                        }
                        else //if ( differenceBetweenGroups <= 0 )
                        {
                            matching1->weight = differenceBetweenGroups * -1; // matching weights is greater, so the weights of the matching minus the different move on
                            winner = matching1;
                            free3groups( matching2, different1, different2);
                        }
                        break;
                    }
                    case 0: // the different weights are of opposite polarity; matching weights are of opposite polarity and cancel each other out
                    {
                        differenceBetweenGroups = different1->weight - different2->weight;
                        if (differenceBetweenGroups > 0)
                        {
                            different1->weight = differenceBetweenGroups; // different1 moves on
                            winner = different1;
                            free3groups( matching1, matching2, different2);
                        }
                        else //if ( differenceBetweenGroups <= 0 )
                        {
                            different2->weight = differenceBetweenGroups * -1; // different2 moves on
                            winner = different2;
                            free3groups( matching1, matching2, different1);
                        }
                        break;
                    }
                }
            }
            else // result1 == 2
            {
                replace4Values(compareArray, different1->group[different1->majorityIndex1], different1->group[different1->majorityIndex2],
                                matching1->group[matching1->majorityIndex1], matching2->group[matching2->majorityIndex1]);
                result2 = QCOUNT(1, compareArray);

                switch ( result2 )
                {
                    case -1: //Error
                        return -1;
                    case 4: // different1, matching1, matching2 are of the same polarity
                    {
                        differenceBetweenGroups = different1->weight + (2*matching1->weight) - different2->weight;
                        if (differenceBetweenGroups > 0)
                        {
                            different1->weight = differenceBetweenGroups; // different1 moves on
                            winner = different1;
                            free3groups( matching1, matching2, different2);
                        }
                        else //if ( differenceBetweenGroups <= 0 )
                        {
                            different2->weight = differenceBetweenGroups * -1; // different2 moves on
                            winner = different2;
                            free3groups( matching1, matching2, different1);
                        }
                        break;
                    }
                    case 2: // matching1 and matching2 are of different polarity; different 1 and 2 are of same polarity
                    {
                        different1->weight = different1->weight + different2->weight; // matching1 and 2 cancel each other out
                        winner = different1;
                        free3groups( matching1, matching2, different2);
                        break;
                    }
                    case 0: // matching1, matching2, different2 are of the same polarity
                    {
                        differenceBetweenGroups = different2->weight + (2*matching1->weight) - different1->weight;
                        if (differenceBetweenGroups > 0)
                        {
                            different2->weight = differenceBetweenGroups; // different2 moves on
                            winner = different2;
                            free3groups( matching1, matching2, different1);
                        }
                        else //if ( differenceBetweenGroups < 0 )
                        {
                            different1->weight = differenceBetweenGroups * -1; // different1 moves on
                            winner = different1;
                            free3groups( matching1, matching2, different2);
                        }
                        break;
                    }
                }
            }
        }
        else //All different weights
        {
            if( result1 == 0 )
            {
                replace4Values( compareArray, group4->group[ group4->majorityIndex1 ], group4->group[ group4->majorityIndex2 ],
                                    group1->group[ group1->majorityIndex1 ], group2->group[ group2->majorityIndex1 ] );
                result2 = QCOUNT(1, compareArray );

                switch( result2 )
                {
                    case -1: //Error
                        return -1;
                    case 4: // Shouldn't occur
                        return -1;
                    case 2: //Group 1 and 2 are of opposite polarity; Group 3 and 4 are of opposite polarity
                    {
                        replace4Values(compareArray, group4->group[group4->majorityIndex1],
                                       group4->group[group4->majorityIndex2],
                                       group2->group[group2->majorityIndex1], group2->group[group2->majorityIndex2]);
                        result3 = QCOUNT(1, compareArray);
                        switch (result3)
                        {
                            case -1: //Error
                                return -1;
                            case 2: //Shouldn't Occur
                                return -1;
                            case 0: //Group 4 and 2 have opposite polarity
                            {
                                differenceBetweenGroups = group4->weight + group1->weight - group3->weight - group2->weight;
                                if (differenceBetweenGroups > 0)
                                {
                                    group4->weight = differenceBetweenGroups;
                                    winner = group4;
                                    free3groups(group1, group2, group3);
                                }
                                else // differenceBetweenGroups <= 0
                                {
                                    group3->weight = differenceBetweenGroups * -1;
                                    winner = group3;
                                    free3groups(group1, group2, group4);
                                }
                                break;
                            }
                            case 4: //Group 4 and 2 are of same polarity
                            {
                                differenceBetweenGroups =
                                        group4->weight + group2->weight - group3->weight - group1->weight;
                                if (differenceBetweenGroups > 0)
                                {
                                    group4->weight = differenceBetweenGroups;
                                    winner = group4;
                                    free3groups(group1, group2, group3);
                                }
                                else // differenceBetweenGroups <= 0
                                {
                                    group3->weight = differenceBetweenGroups * -1;
                                    winner = group3;
                                    free3groups(group1, group2, group4);
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case 0: // Group 1 and 2 are of same polarity; Group 3 and 4 are of same polarity
                    {
                        // Since 4 is greater than 2 and 3 is greater than 1, 3+4 is automatically the winner
                        group4->weight = group4->weight + group3->weight - group1->weight - group2->weight;
                        winner = group4;
                        free3groups(group1, group2, group3);
                        break;
                    }
                }
            }
            else //result1 == 2
            {
                replace4Values( compareArray, group4->group[ group4->majorityIndex1 ], group4->group[ group4->majorityIndex2 ],
                                    group1->group[ group1->majorityIndex1 ], group2->group[ group2->majorityIndex1] );
                result2 = QCOUNT( 1, compareArray );

                switch (result2)
                {
                    case -1: //Error
                        return -1;
                    case 0: // Group 1, 2 and 3 of same polarity; Group 4 of opposite polarity
                    {
                        differenceBetweenGroups = group4->weight - group1->weight - group2->weight - group3->weight;
                        if( differenceBetweenGroups > 0 )
                        {
                            group4->weight = differenceBetweenGroups;
                            winner = group4;
                            free3groups( group1, group2, group3 );
                        }
                        else //differenceBetweenGroups <= 0
                        {
                            group3->weight = differenceBetweenGroups * -1;
                            winner = group3;
                            free3groups( group1, group2, group4 );
                        }
                        break;
                    }
                    case 2: // Group 1 and 2 of opposite polarity; Group 3 and 4 of same polarity
                    {
                        replace4Values(compareArray, group4->group[group4->majorityIndex1], group4->group[group4->majorityIndex2],
                                       group2->group[group2->majorityIndex1], group2->group[group2->majorityIndex2]);
                        result3 = QCOUNT(1, compareArray);
                        switch (result3)
                        {
                            case -1: //Error
                                return -1;
                            case 2: //Shouldn't Occur
                                return -1;
                            case 0: //Group 4 and 2 have opposite polarity
                            {
                                group4->weight = group4->weight + group3->weight + group1->weight - group2->weight;
                                winner = group4;
                                free3groups(group1, group2, group3);
                                break;
                            }
                            case 4: //Group 4 and 2 are of same polarity; Group 1 is the opposite polarity
                            {
                                group4->weight = group4->weight + group3->weight + group2->weight - group1->weight;
                                winner = group4;
                                free3groups(group1, group2, group3);
                                break;
                            }
                        }
                        break;
                    }
                    case 4: //Group 1, 2 and 4 are of same polarity; Group 3 is the opposite polarity
                    {
                        group4->weight = group4->weight + group2->weight + group1->weight - group3->weight;
                        winner = group4;
                        free3groups(group1, group2, group3);
                        break;
                    }
                }
            }
        }
    }

    //Found winner in above code; Will return early if no winner is found
    if( winner->weight == 0)
        free( winner );
    else
        placeBasedOffWeight( knownGroups, winner);

    return 0;
}

// Narrows down both of the given list of groups until at most one group remains
// Returns -1 on error; 0 normally
int runTournament( linkListEnds *knownGroups, linkListEnds *unknownGroups )
{
    if( unknownGroups->length <= 1 && knownGroups->length == 0 )
        return 0;

    while( unknownGroups->length > 0 ) //Don't want any unknowns
        if( (filterOutUnknowns( knownGroups,unknownGroups )) == -1 )
            return -1;

    if( knownGroups->length <= 1 )
        return 0;

    while( knownGroups->length > 1 ) //Want at most 1 known
        if( (compareKnowns( knownGroups )) == -1 )
            return -1;

    return 0;

}

//Will determine the correct output of musub
//Returns 0 if no majority; # > 0 if majority index exists; -1 on error
int tournamentResults( linkListEnds *knownGroups, linkListEnds *unknownGroups, linkListEnds *firstOf, queryGroup *oddGroup)
{
    //Reminder: oddGroup's weight is not based off majority; Only on size, which is n%4
    int compareArray[4];
    int result1, result2;
    queryGroup* referenceGroup;

    //printf("Done!\n");
    if ( unknownGroups->length == 1 ) //Can only have a weight of 2 and can only occur when knownGroup->length == 0
    {
        //printf( "fW-%d fE-%d uGw-%d uGE-%d\n", firstOf->start->weight, firstOf->start->group[0], unknownGroups->start->weight, unknownGroups->start->group[0] );
        referenceGroup = firstOf->start; //This cannot be the same the unknownGroup by design
        replace4Values( compareArray, unknownGroups->start->group[0], unknownGroups->start->group[1],
                        referenceGroup->group[0], referenceGroup->group[1] );
        if( (result1 = QCOUNT( 1, compareArray )) == -1 )
            return -1; //Error
        compareArray[0] = unknownGroups->start->group[2];
        if( (result2 = QCOUNT( 1, compareArray )) == -1 )
            return -1; //Error
        //printf( "r1-%d r2-%d\n", result1, result2);
        if( result1 == result2 )
        {
            unknownGroups->start->majorityIndex1 = 0;
            unknownGroups->start->majorityIndex2 = 2;
        }
        else
        {
            unknownGroups->start->majorityIndex1 = 1;
            unknownGroups->start->majorityIndex2 = 3;
        }
        addToLinkList( knownGroups, popFront(unknownGroups) );
    }

    if( knownGroups->length == 1 ) //If case, there should only be 1 group left and it is in knownGroup
    {
        //printf( "kW-%d m1-%d m2-%d oW-%d\n", knownGroups->start->weight, knownGroups->start->majorityIndex1, knownGroups->start->majorityIndex2, oddGroup->weight );
        if( knownGroups->start->weight > oddGroup->weight ) //aka does oddGroup even matter?
            return knownGroups->start->group[ knownGroups->start->majorityIndex1 ];
        else //Should only be the case when known has a weight of 2 and oddGroup has a weight of 2/3
        {
            replace4Values( compareArray, oddGroup->group[0], oddGroup->group[1],
                            knownGroups->start->group[ knownGroups->start->majorityIndex1 ],
                            knownGroups->start->group[ knownGroups->start->majorityIndex2 ]);
            result1 = QCOUNT(1, compareArray );
            if( result1 > 0 ) //aka can oddGroup change results
                return knownGroups->start->group[ knownGroups->start->majorityIndex1 ];
            else if ( result1 == -1 )
                return -1; //Error
            else
            {
                if( oddGroup->weight == 2 ) //If true, they cancel each-other out; If false, the tie-breaker is oddGroup->group[2]
                    return 0;
                else
                    return oddGroup->group[2];
            }
        }
    }
    else //Occurs when knownGroups and unknownGroups are empty; Either 0 or majority of oddGroup
    {
        if( firstOf->end->weight == 4 ) //4 requires only 1 comparison for oddGroup; 0/2 require 2
            referenceGroup = firstOf->end;
        else //Will have either a weight of 0 or 2
            referenceGroup = firstOf->start;

        switch ( oddGroup->weight )
        {
            case 0:
                return 0;
            case 1:
                return oddGroup->group[0];
            case 2:
            {
                if( referenceGroup->weight == 4 )
                {
                    replace4Values( compareArray, oddGroup->group[0], oddGroup->group[1],
                                    referenceGroup->group[ referenceGroup->majorityIndex1 ],
                                    referenceGroup->group[ referenceGroup->majorityIndex2 ]);
                    if ( (result1 = QCOUNT(1, compareArray)) == 2 )
                        return 0;
                    else if ( result1 == -1 )
                        return -1; //Error
                    else
                        return oddGroup->group[0];
                }
                else
                {
                    replace4Values( compareArray, oddGroup->group[0], referenceGroup->group[0],
                                    referenceGroup->group[1], referenceGroup->group[2] );
                    if( (result1 = QCOUNT( 1, compareArray )) == -1 )
                        return -1; //Error
                    compareArray[0] = oddGroup->group[1];
                    if( (result2 = QCOUNT( 1, compareArray )) == -1 )
                        return -1; //Error
                    if( result1 == result2 ) //aka same value in both group[0] and group[1]
                        return oddGroup->group[0];
                    else
                        return  0;
                }
                break;
            }
            case 3:
            {
                if( referenceGroup->weight == 4)
                {
                    replace4Values( compareArray, oddGroup->group[0], oddGroup->group[1],
                                    referenceGroup->group[ referenceGroup->majorityIndex1 ],
                                    referenceGroup->group[ referenceGroup->majorityIndex2 ]);
                    if ( (result1 = QCOUNT(1, compareArray)) == 2 ) //aka different value for group[1] and group[1]
                        return oddGroup->group[2];
                    else if ( result1 == -1)
                        return -1; //Error
                    else
                        return oddGroup->group[0];
                }
                else
                {
                    replace4Values( compareArray, oddGroup->group[0], referenceGroup->group[0],
                                    referenceGroup->group[1], referenceGroup->group[2] );
                    if( (result1 = QCOUNT( 1, compareArray )) == -1 )
                        return -1; //Error
                    compareArray[0] = oddGroup->group[1];
                    if( (result2 = QCOUNT( 1, compareArray )) == -1 )
                        return -1; //Error
                    if( result1 == result2 ) //aka same value in both group[0] and group[1]
                        return oddGroup->group[0];
                    else
                        return oddGroup->group[2];
                }
                break;
            }
        }
    }
}

//Will return the element representing the majority or 0 if no majority
//Returns -1 on error; If no error message printed directly from this program, the error comes from QCOUNT failing for some reason
int mysub( int n )
{
    if ( n < 10 || n > 10000 )
    {
        printf("Invalid input given to mysub\n");
        return -1;
    }

    linkListEnds knownGroups, unknownGroups; //Known groups are where we know the majority; Unknown are where we don't
    linkListEnds firstOf; //Will contain the first appearance of a 4, 2 , and 0 group
    queryGroup oddGroup; //Contains the group resulting from the last (n%4) #s

    if ( setup( n, &knownGroups, &unknownGroups, &firstOf, &oddGroup) == -1 )
        return -1;
    if( runTournament( &knownGroups, &unknownGroups) == -1 )
        return -1;

    return tournamentResults( &knownGroups, &unknownGroups, &firstOf, &oddGroup);
}