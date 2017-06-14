#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef int BOOL;
#define FALSE 0
#define TRUE 1

typedef struct _grayImage {
	unsigned short rows, cols;
	unsigned char** pixels;
}grayImage;

typedef unsigned short imgPos[2];

typedef struct _treeNode {
	imgPos position;
	struct _treeNodeListCell *next_possible_positions;//list of Locations
}treeNode;

typedef struct _treeNodeListCell {
	treeNode* node;
	struct _treeNodeListCell* next;
}treeNodeListCell;

typedef struct _segment {
	treeNode* root;
}Segment;

static int** signMat;//goes to header

void checkMemoryAllocation(void* ptr);
grayImage* readPGM(char* fname);
Segment findSingleSegment(grayImage* img, imgPos start, unsigned char threshold);
int** createSignMat(grayImage* img, imgPos start, unsigned char threshold);
void updateSignMatWithValues(int** signMat, unsigned char maxGrayValue, unsigned char minGrayValue, grayImage* img);
int countNeighbors(imgPos start, grayImage* img, int** signMatrix, unsigned char threshold);
imgPos* NextPossibleNeighbors(imgPos start, grayImage* img, int** signMatrix, unsigned char threshold);
treeNodeListCell* findSingleSegmentREC(grayImage* img, imgPos originalStart, imgPos start, unsigned char threshold, int** signMat);
//treeNodeListCell* makeNeighbourList(grayImage* img, imgPos start, unsigned char threshold, int** signMat);
treeNodeListCell* createListNode(imgPos start, treeNodeListCell* next);
void printSegmentList(treeNode* headOfSegment);


void main()
{
	extern int** signMat;
	int i, j;
	grayImage* res;
	Segment test;
	res = readPGM("testPGM.pgm");
	imgPos start;
	start[0] = 0;
	start[1] = 0;

	test = findSingleSegment(res, start,0);

	printSegmentList(test.root);

	for (i = 0; i < res->rows; i++)
	{
		for (j = 0; j < res->cols; j++)
		{
			printf("%d ", res->pixels[i][j]);
		}
		printf("\n");
	}

	for (i = 0; i < res->rows; i++)
	{
		for (j = 0; j < res->cols; j++)
		{
			printf("%d ", signMat[i][j]);
		}
		printf("\n");
	}

	system("pause");
}
void checkMemoryAllocation(void* ptr)
{
	if (ptr == NULL)
	{
		printf("Memory Allocation Failed!");
		exit(1);
	}
}

void printSegmentList(treeNode* headOfSegment)
{
	treeNodeListCell* curr, *currOfcurr;
	int i = 1;

	printf("Head Of Segment Is: [%d,%d]\n", headOfSegment->position[0], headOfSegment->position[1]);
	curr = headOfSegment->next_possible_positions;
	while (curr != NULL)
	{
		printf("Pixel %d is: [%d,%d]\n", i, curr->node->position[0], curr->node->position[1]);
		currOfcurr = curr->node->next_possible_positions;
		printf("		Next Possible Positions:->");
		while (currOfcurr != NULL)
		{
			printf("[%d,%d]->", currOfcurr->node->position[0], currOfcurr->node->position[1]);
			currOfcurr = currOfcurr->next;
		}
		if (currOfcurr == NULL)
			printf("NULL.\n");
		i++;
		curr = curr->next;
	}

}

//grayImage
grayImage* readPGM(char* fname)
{
	//variables
	int i, j, k;
	int temp = 0;
	char** mat;
	FILE* fp;
	char line[257];
	char line2[257];
	grayImage* res;
	BOOL end = FALSE;
	char insertValue;
	res = (grayImage*)malloc(sizeof(grayImage)); //allocate grayImage 
	fp = fopen(fname, "r");

	fgets(line, 256, fp); //read and ignore from the first line 'P2'
	fscanf(fp, "%hu %hu", &res->cols, &res->rows);//may need to change the order of rows and cols
	fgets(line, 256, fp);
	fgets(line, 256, fp); //read and ignore from the first line 'max gray value'

						  //allocate the pixels matrix
	mat = (char**)malloc((res->rows) * sizeof(char*));
	for (k = 0; k < res->cols; k++)
		mat[k] = (char*)malloc((res->cols) * sizeof(char));

	//filling the matrix with values from the file

	for (i = 0; i < res->rows; i++)
	{
		for (j = 0; j < res->cols; j++)
		{
			fscanf(fp, "%d", &temp);
			insertValue = temp;//int to char convert
			mat[i][j] = insertValue;
		}
		fgets(line, 256, fp);
	}
	res->pixels = mat;

	return res;
}

Segment findSingleSegment(grayImage* img, imgPos start, unsigned char threshold)
{
	Segment res;
	treeNode* headOfSegment;
	treeNodeListCell* tmp;
	extern int** signMat;
	unsigned char grayValue, maxGrayValue, minGrayValue;
	int i, j;

	signMat = createSignMat(img, start, threshold);
	grayValue = (char)img->pixels[start[0]][start[1]];
	maxGrayValue = grayValue + threshold;
	minGrayValue = grayValue - threshold;
	//	updateSignMatWithValues(signMat, maxGrayValue, minGrayValue, img);

	headOfSegment = (treeNode*)malloc(sizeof(treeNode));
	headOfSegment->position[0] = start[0];
	headOfSegment->position[1] = start[1];

	tmp = findSingleSegmentREC(img, start, start, threshold, signMat);
	headOfSegment->next_possible_positions = tmp;
	res.root = headOfSegment;


	return res;
}

treeNodeListCell* findSingleSegmentREC(grayImage* img, imgPos originalStart, imgPos start, unsigned char threshold, int** signMat)
{
	treeNodeListCell *startNode, *subList;
	imgPos* newStart, *startKochav;
	int neighCount;

	if (start == NULL)
		neighCount = 0;
	else
	{
		signMat[start[0]][start[1]] = 1;
		neighCount = countNeighbors(originalStart, img, signMat, threshold);
	}

	if (neighCount == 0)
		return NULL;
	else
	{
		newStart = NextPossibleNeighbors(originalStart, img, signMat, threshold);
		subList = findSingleSegmentREC(img, originalStart, *newStart, threshold, signMat);
		startNode = createListNode(*newStart, subList);
		startNode->node->next_possible_positions = findSingleSegmentREC(img, *newStart, *newStart, threshold, signMat);
	}
	return startNode;

}

treeNodeListCell* createListNode(imgPos start, treeNodeListCell* next)
{
	treeNodeListCell* res;
	res = (treeNodeListCell*)malloc(sizeof(treeNodeListCell));
	res->node = (treeNode*)malloc(sizeof(treeNode));
	res->node->position[0] = start[0];
	res->node->position[1] = start[1];
	res->next = next;
}
/*
treeNode* findSingleSegmentREC(grayImage* img, imgPos start, unsigned char threshold, int** signMat)
{
	treeNode* segmentRoot;
	if (img->pixels == NULL)
		return NULL;
	else
	{
		segmentRoot = (treeNode*)malloc(sizeof(treeNode));
		segmentRoot->position[0] = start[0];
		segmentRoot->position[1] = start[1];
		signMat[start[0]][start[1]] = 1;
		segmentRoot->next_possible_positions = makeNeighbourList(img, start, threshold, signMat);

	}
	return segmentRoot;
}


treeNodeListCell* makeNeighbourList(grayImage* img, imgPos start, unsigned char threshold, int** signMat)
{
	imgPos* newStart;
	treeNodeListCell* res;

	res = (treeNodeListCell*)malloc(sizeof(treeNodeListCell));

	while (countNeighbors(start, img, signMat)!=0)
	{
		newStart = NextPossibleNeighbors(start, img, signMat);
		res->node = findSingleSegmentREC(img, *newStart, threshold, signMat);
		res->next= (treeNodeListCell*)malloc(sizeof(treeNodeListCell));
		res = res->next;
	}

	if (countNeighbors(start, img, signMat) == 0)
		return res;

}


*/

int** createSignMat(grayImage* img, imgPos start, unsigned char threshold)
{
	int** res;
	int cols, rows;
	int i;
	cols = img->cols;
	rows = img->rows;

	res = (int**)malloc(rows * sizeof(int*));
	checkMemoryAllocation(res);
	for (i = 0; i < rows; i++)
	{
		res[i] = (int*)calloc(cols, sizeof(int));
		checkMemoryAllocation(res[i]);
	}

	return res;
}
//consider delete function
void updateSignMatWithValues(int** signMat, unsigned char maxGrayValue, unsigned char minGrayValue, grayImage* img)
{
	int i, j;
	int rows, cols;
	rows = img->rows;
	cols = img->cols;

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			if ((img->pixels[i][j] < minGrayValue) || (img->pixels[i][j] > maxGrayValue))
				signMat[i][j] = 1;
		}
	}
}


int countNeighbors(imgPos start, grayImage* img, int** signMatrix, unsigned char threshold)
{
	int counter = 0;

	//Start checking the matrix from the top-left corner->top-right corner->top row->bottom-right->right  column
	//->bottom row->bottom-left column->left column.
	//1. (0,0)-
	if (start[0] == 0 && start[1] == 0)
	{
		if (fabs((double)img->pixels[0][1] - img->pixels[0][0]) <= threshold)
			if (signMatrix[0][1] == 0)
				counter++;
		if (fabs((double)img->pixels[1][1] - img->pixels[0][0]) <= threshold)
			if (signMatrix[1][1] == 0)
				counter++;
		if (fabs((double)img->pixels[1][0] - img->pixels[0][0]) <= threshold)
			if (signMatrix[1][0] == 0)
				counter++;
	}

	//2.(0,cols-1)
	else if (start[0] == 0 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[1][img->cols - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][img->cols - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[1][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][img->cols - 2] == 0)
				counter++;
		if (fabs((double)img->pixels[0][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][img->cols - 2] == 0)
				counter++;
	}

	//3.First Row(0,?)
	else if (start[0] == 0)
	{
		if (fabs((double)img->pixels[0][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[1][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[1][start[1]] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1]] == 0)
				counter++;
		if (fabs((double)img->pixels[1][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1] - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[0][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][start[1] - 1] == 0)
				counter++;
	}

	//4.Bottom-right corner - (rows-1,cols-1)
	else if (start[0] == img->rows - 1 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][img->cols - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][img->cols - 2] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][img->cols - 2] == 0)
				counter++;
	}

	//5. Right column - (?, cols-1)
	else if (start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][img->cols - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][img->cols - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][img->cols - 2] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0]][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][img->cols - 2] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][img->cols - 2] == 0)
				counter++;
	}

	//6. Bottom-left corner - (rows-1,0)
	else if (start[0] == img->rows - 1 && start[1] == 0)
	{
		if (fabs((double)img->pixels[img->rows - 2][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][0] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 2][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][1] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][1] == 0)
				counter++;
	}

	//7. Bottom row - (rows-1,?)
	else if (start[0] == img->rows - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1]] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 2][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][start[1] - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[img->rows - 2][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1] - 1] == 0)
				counter++;
	}

	//8. Left column - (?,0)
	else if (start[1] == 0)
	{
		if (fabs((double)img->pixels[start[0] - 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][0] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0]][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][0] == 0)
				counter++;
	}

	else
	{
		if (fabs((double)img->pixels[start[0] - 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1]] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0]][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1] + 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1]] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] + 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1] - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0]][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][start[1] - 1] == 0)
				counter++;
		if (fabs((double)img->pixels[start[0] - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1] - 1] == 0)
				counter++;
	}

	return counter;
}

imgPos* NextPossibleNeighbors(imgPos start, grayImage* img, int** signMatrix, unsigned char threshold)
{
	imgPos* res;
	res = (imgPos*)malloc(sizeof(imgPos));

	//Start checking the matrix from the top-left corner->top-right corner->top row->bottom-right->right  column
	//->bottom row->bottom-left column->left column.
	//1. (0,0)-
	if (start[0] == 0 && start[1] == 0)
	{
		if (fabs((double)img->pixels[0][1] - img->pixels[0][0]) <= threshold)
			if (signMatrix[0][1] == 0)
			{
				(*res)[0] = 0;
				(*res)[1] = 1;
				return res;
			}
		if (fabs((double)img->pixels[1][1] - img->pixels[0][0]) <= threshold)
			if (signMatrix[1][1] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = 1;
				return res;

			}
		if (fabs((double)img->pixels[1][0] - img->pixels[0][0]) <= threshold)
			if (signMatrix[1][0] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = 0;
				return res;
			}
	}

	//2.(0,cols-1)
	else if (start[0] == 0 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[1][img->cols - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][img->cols - 1] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = img->cols - 1;
				return res;
			}
		if (fabs((double)img->pixels[1][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][img->cols - 2] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = img->cols - 2;
				return res;
			}
		if (fabs((double)img->pixels[0][img->cols - 2] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][img->cols - 2] == 0)
			{
				(*res)[0] = 0;
				(*res)[1] = img->cols - 2;
				return res;
			}
	}

	//3.First Row(0,?)
	else if (start[0] == 0)
	{
		if (fabs((double)img->pixels[0][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][start[1] + 1] == 0)
			{

				(*res)[0] = 0;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[1][start[1] + 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1] + 1] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[1][start[1]] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1]] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = start[1];
				return res;
			}
		if (fabs((double)img->pixels[1][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[1][start[1] - 1] == 0)
			{
				(*res)[0] = 1;
				(*res)[1] = start[1] - 1;
				return res;
			}
		if (fabs((double)img->pixels[0][start[1] - 1] - img->pixels[0][start[1]]) <= threshold)
			if (signMatrix[0][start[1] - 1] == 0)
			{
				(*res)[0] = 0;
				(*res)[1] = start[1] - 1;
				return res;
			}
	}

	//4.Bottom-right corner - (rows-1,cols-1)
	else if (start[0] == img->rows - 1 && start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][img->cols - 1] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = img->cols - 1;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][img->cols - 2] == 0)
			{

				(*res)[0] = img->rows - 1;
				(*res)[1] = img->cols - 2;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 2][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][img->cols - 2] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = img->cols - 2;
				return res;
			}
	}

	//5. Right column - (?, cols-1)
	else if (start[1] == img->cols - 1)
	{
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][img->cols - 1] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = img->cols - 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][img->cols - 1] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = img->cols - 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][img->cols - 2] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = img->cols - 2;
				return res;
			}
		if (fabs((double)img->pixels[start[0]][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][img->cols - 2] == 0)
			{
				(*res)[0] = start[0];
				(*res)[1] = img->cols - 2;
				return res;
			}
		if (fabs((double)img->pixels[start[0] - 1][img->cols - 2] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][img->cols - 2] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = img->cols - 2;
				return res;
			}
	}

	//6. Bottom-left corner - (rows-1,0)
	else if (start[0] == img->rows - 1 && start[1] == 0)
	{
		if (fabs((double)img->pixels[img->rows - 2][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][0] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = 0;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 2][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][1] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = 1;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][1] == 0)
			{
				(*res)[0] = img->rows - 1;
				(*res)[1] = 1;
				return res;
			}
	}

	//7. Bottom row - (rows-1,?)
	else if (start[0] == img->rows - 1)
	{
		if (fabs((double)img->pixels[img->rows - 2][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1]] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = start[1];
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 2][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1] + 1] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][start[1] + 1] == 0)
			{
				(*res)[0] = img->rows - 1;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 1][start[1] - 1] == 0)
			{
				(*res)[0] = img->rows - 1;
				(*res)[1] = start[1] - 1;
				return res;
			}
		if (fabs((double)img->pixels[img->rows - 2][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[img->rows - 2][start[1] - 1] == 0)
			{
				(*res)[0] = img->rows - 2;
				(*res)[1] = start[1] - 1;
				return res;
			}
	}

	//8. Left column - (?,0)
	else if (start[1] == 0)
	{
		if (fabs((double)img->pixels[start[0] - 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][0] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = 0;
				return res;
			}
		if (fabs((double)img->pixels[start[0] - 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][1] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0]][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][1] == 0)
			{
				(*res)[0] = start[0];
				(*res)[1] = 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][1] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][0] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][0] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = 0;
				return res;
			}
	}

	else
	{
		if (fabs((double)img->pixels[start[0] - 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1]] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = start[1];
				return res;
			}
		if (fabs((double)img->pixels[start[0] - 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1] + 1] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0]][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][start[1] + 1] == 0)
			{
				(*res)[0] = start[0];
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][start[1] + 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1] + 1] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = start[1] + 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][start[1]] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1]] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = start[1];
				return res;
			}
		if (fabs((double)img->pixels[start[0] + 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] + 1][start[1] - 1] == 0)
			{
				(*res)[0] = start[0] + 1;
				(*res)[1] = start[1] - 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0]][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0]][start[1] - 1] == 0)
			{
				(*res)[0] = start[0];
				(*res)[1] = start[1] - 1;
				return res;
			}
		if (fabs((double)img->pixels[start[0] - 1][start[1] - 1] - img->pixels[start[0]][start[1]]) <= threshold)
			if (signMatrix[start[0] - 1][start[1] - 1] == 0)
			{
				(*res)[0] = start[0] - 1;
				(*res)[1] = start[1] - 1;
				return res;
			}
	}

	return NULL;

}