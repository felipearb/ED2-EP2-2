#include "headers.h"

// Creates a new B-tree or opens an existing B-tree from the file
// Parameters:
// - fileName: The name of the file containing the B-tree
// - mode: The mode for opening the file, true for creating a new file, false for opening an existing file
// Returns:
// - A pointer to the created or opened B-tree
bTree* createTree(char* fileName,bool mode)
{
	bTree* tree = malloc(sizeof(bTree));
    
	if(!mode) //new filea
    {
        
        strcpy(tree->fileName,fileName);
        tree->fp = fopen(fileName,"w");
        fclose(tree->fp);

        tree->root = 0;
        tree->nextPos = 0;
    } else {
        FILE *oldFile = fopen("tree.dat", "r");
        fread(tree, sizeof(bTree), 1, oldFile);
        fclose(oldFile);
    }

    tree->fp = fopen(fileName, "r+");
    return tree;
}

bTreeNode* nodeInit(bTreeNode* node,bool isLeaf,bTree* tree)
{
	node->isLeaf = isLeaf;
	node->noOfRecs=0;
	node->pos = tree->nextPos;
	(tree->nextPos)++;
	int i;
	for (int i = 0; i < 2*t; ++i)
	{
		node->children[i] = -1;
	}
	return node;
}

// Writes a B-tree node to the file
// Parameters:
// - ptr_tree: Pointer to the B-tree
// - p: The B-tree node to be written
// - pos: Position of the node in the file
void writeFile(bTree* ptr_tree, bTreeNode* p, int pos) {
    if(pos == -1) {
        pos = ptr_tree->nextPos++;
    }

    fseek(ptr_tree->fp, pos * sizeof(bTreeNode), 0);
    fwrite(p, sizeof(bTreeNode), 1, ptr_tree->fp);
    
}

// Reads a B-tree node from the file
// Parameters:
// - ptr_tree: Pointer to the B-tree
// - p: The B-tree node to be read
// - pos: Position of the node in the file
void readFile(bTree* ptr_tree, bTreeNode* p, int pos) {    
    //Copia as informações de ptr_tree(memoria secundaria) para p)memoria principal)
    fseek(ptr_tree->fp, pos * sizeof(bTreeNode), SEEK_SET);
    fread(p, sizeof(bTreeNode), 1, ptr_tree->fp);
}

// Enters data into a record node
// Parameters:
// - record: The record node to be filled with data
// - valid: valid of the record
// - codigoLivro: codigoLivro of the record
// - titulo: titulo of the record
// - nome: nomeCompletoPrimeiroAutor of the record
// - ano: anoPublicacao of the record
void enterData(recordNode* record, bool valido,int codigoLivro, char titulo[], char nome[], int ano){
    record->valid = valido;
    record->codigoLivro = codigoLivro;
    strncpy(record->titulo, titulo,30);
    strncpy(record->nomeCompletoPrimeiroAutor, nome,30);
    record->anoPublicacao = ano;
    return;
}

recordNode* getData(char *filepath, int len) {
    
    recordNode *children = malloc(sizeof(recordNode)*len);
    char delim = ','; char line[256];
    char valid;
    int file_no = 0;
    int i,codigo,ano;

    char titulo[30];
    char nome[30];
    
    FILE *inpFile = fopen(filepath, "r");
    
    while(file_no < len && fscanf(inpFile, "%c,%d,%[^,],%[^,],%d,%d",valid &codigo, titulo, nome, &ano)) {
        enterData(&children[file_no],valid,codigo,titulo,nome,ano);
        file_no++;
    }

    return children;
}

// Splits a child node during insertion
// Parameters:
// - tree: The B-tree
// - x: The parent node that needs to be split
// - i: The index of the child node to be split
// - y: The child node to be split
void splitChild(bTree* tree, bTreeNode* x, int i, bTreeNode* y)
{
	bTreeNode* z = malloc(sizeof(bTreeNode));
	nodeInit(z,y->isLeaf,tree);
	z->noOfRecs = t-1;

	int j;
	for(j=0;j<t-1;j++)
	{
		z->children[j] = y->children[j+t];
	}

	if(!y->isLeaf)
	{
		for(j=0;j<t;j++)
		{
			z->children[j] = y->children[j+t];
            y->children[j+t] = -1; 
		}
	}
	y->noOfRecs = t-1;

	for(j=(x->noOfRecs); j >= i+1;j--)
	{
		x->children[j+1] = x->children[j];
	}
	
	x->children[i+1] = z->pos;

	for(j=(x->noOfRecs) - 1; j >= i;j--)
	{
		x->children[j+1] = x->children[j];
	}
	x->children[i] = y->children[t-1];
	x->noOfRecs++;

    writeFile(tree, x, x->pos);
    writeFile(tree, y, y->pos);
    writeFile(tree, z, z->pos);
	free(z);
}

void insertNonFull(bTree* tree,bTreeNode* x,recordNode* record)
{	
	int i = (x->noOfRecs)-1;
	if(x->isLeaf == true)
	{
		while((i>=0) && (record->codigoLivro < x->children[i]->codigoLivro))
		{
			x->children[i+1] = x->children[i];
			i--;
		}
		x->children[i+1] = record;
		(x->noOfRecs)++;

        writeFile(tree, x, x->pos);
	}
	else
	{
		while((i>=0) && (record->codigoLivro < x->children[i]->codigoLivro))
		{
			i=i-1;
		}
		bTreeNode* childAtPosi = malloc(sizeof(bTreeNode));
        readFile(tree, childAtPosi, x->children[i+1]);
        
		if(childAtPosi->noOfRecs == (2*t-1))
		{
			splitChild(tree,x,i+1,childAtPosi);
			if( x->children[i+1]->codigoLivro < record->codigoLivro){
				i++;
			}
		}

        readFile(tree, childAtPosi, x->children[i+1]);
		insertNonFull(tree,childAtPosi,record);

		free(childAtPosi);
	}
}

// Inserts a record into the B-tree
// Parameters:
// - tree: The B-tree to insert the record into
// - record: The record to be inserted
// Description:
// This function inserts a record into the B-tree. It starts by finding the appropriate
// leaf node to insert the record. If the leaf node is not full, the record is directly
// inserted. If the leaf node is full, a split operation is performed to create a new
// leaf node and redistribute the records. The split operation may propagate upwards if
// necessary to maintain the B-tree properties.
void insert(bTree* tree,recordNode* record)
{
	if(tree->nextPos == 0) 
	{
		tree->root = tree->nextPos;

		bTreeNode* firstNode = malloc(sizeof(bTreeNode));
		nodeInit(firstNode,true,tree);
		firstNode->children[0] = record;
		(firstNode->noOfRecs)++;

        writeFile(tree, firstNode, firstNode->pos);

		free(firstNode);
		return;
	}
	else
	{
		bTreeNode* rootCopy = malloc(sizeof(bTreeNode));
        readFile(tree, rootCopy, tree->root);

		if(rootCopy->noOfRecs == (2*t-1))
		{
			bTreeNode* newRoot = malloc(sizeof(bTreeNode));
			nodeInit(newRoot,false,tree);
			newRoot->children[0] = tree->root;

			splitChild(tree,newRoot,0,rootCopy);

			int i=0;
			if(newRoot->children[0]->codigoLivro < record->codigoLivro){
				i++;
			}
			
			bTreeNode* childAtPosi = malloc(sizeof(bTreeNode));
            readFile(tree, childAtPosi, newRoot->children[i]);
			insertNonFull(tree,childAtPosi,record);

			tree->root = newRoot->pos;
            
            

			free(newRoot);
            free(childAtPosi);
		}
		else
		{
			insertNonFull(tree,rootCopy,record);
		}
		free(rootCopy);
	}
}

// Traverses the B-tree and prints the nodes
// Parameters:
// - tree: The B-tree to be traversed and printed
// - root: The root node of the B-tree
// Description:
// This function traverses the B-tree in a depth-first manner and prints the details of
// each node encountered. It starts from the root node and recursively visits each child
// node, printing their details. The traversal helps visualize the structure of the B-tree.
void traverse(bTree* tree, int root) {
    /*
    le da memória secundária as informações contidas no nó  bTree* tree e as passas para a v
    ariável toPrint(tipo bTree*,contida em memória principal) ,que é utilizada pela função 
    dispNode que printa as informações contidas em to print. Esse processo é feito de forma 
    recursiva pela função traverse de forma a ser executado no primeiro nó passado e seus 
    descendentes.
    */
    if(-1 == root) {//se o nó não existe, retorna
        return;
    }

    bTreeNode *toPrint = malloc(sizeof(bTreeNode));//Aloca toPrint na memoria
    readFile(tree, toPrint, root);//copia as informações do nó tree para toPrint
    dispNode(toPrint);//Printa as principais informações de toPrint
    
    for(int i = 0; i < 2*t; i++) {//realiza a mesma operação de forma recursiva para os nós filhos
        traverse(tree, toPrint->children[i]);
    }

    free(toPrint);//libera a memoria /
}

// Prints the details of a B-tree node
// Parameters:
// - node: The B-tree node to be displayed
// Description:
// This function prints the details of a B-tree node, including the keys and pointers to
// child nodes. It helps in understanding the internal structure of the B-tree and its organization.
void dispNode(bTreeNode* node)
{
    /*A função printa as seguintes propiedas do nó:Posição,quantidade de registros,se é folha, 
    as chaves dos registros(em ordem) e as posições dos filhos(-1 se não existe o filho).
    */
	printf("Position in node:%d\n",node->pos );//Posição do nó
    printf("Number of Records:%d\n",node->noOfRecs );//quantidade de registros
	printf("Is leaf?:%d\n",node->isLeaf );//se é folha
	printf("Keys:\n");
	for(int i = 0; i < node->noOfRecs; i++)//printa as chaves dos registros
	{
		printf("%d ", node->children[i]->codigoLivro);
	}
	printf("\nLinks:\n");
	for (int i = 0; i < 2*t; ++i)//printa a posição dos nós filhos
	{
        printf("%d ",node->children[i] );
	}
	printf("\n\n");
}

recordNode* searchRecursive(bTree* tree, int codigoLivro, bTreeNode* root) {
    int i = 0;
    
    while(i < root->noOfRecs && codigoLivro > root->children[i]->codigoLivro)
        i++;
    
    
    if(i < root->noOfRecs && codigoLivro == root->children[i]->codigoLivro)
        return root->children[i];
    
    
    else if(root->isLeaf) {
        return NULL;
    }
    else {
        bTreeNode* childAtPosi = malloc(sizeof(bTreeNode));
        readFile(tree, childAtPosi, root->children[i]);

        recordNode* found = searchRecursive(tree, codigoLivro, childAtPosi);
        free(childAtPosi);
        return found;
    }
}


// Searches for a record in the B-tree
// Parameters:
// - tree: The B-tree
// - key: The key of the record to be searched
// Returns:
// - The record with the specified key, or NULL if not found
// Description:
// This function searches for a record with the specified key in the B-tree. It starts
// from the root node and follows the appropriate child nodes based on the key values.
// If the key is found in a leaf node, the corresponding record is returned. If the key
// is not found, NULL is returned.
recordNode* search(bTree* tree, int codigoLivro) {
    
    bTreeNode* root = malloc(sizeof(bTreeNode));
    readFile(tree, root, tree->root);

    recordNode* result = searchRecursive(tree, codigoLivro, root);
    free(root);
    return result;
     
}


// Finds the index of a key in a B-tree node
// Parameters:
// - node: The B-tree node
// - k: The key to be searched
// Returns:
// - The index of the key in the node, or -1 if not found
// Description:
// This function finds the index of a key in a B-tree node. It performs a binary search
// to locate the position of the key within the node. If the key is found, its index is
// returned. Otherwise, -1 is returned to indicate that the key is not present in the node.
int findKey(bTreeNode* node, int k) {
    int idx=0;
    while (idx < node->noOfRecs && node->children[idx]->codigoLivro < k)
        ++idx;
    return idx;
}


// Removes a record from a leaf node
// Parameters:
// - tree: The B-tree
// - node: The leaf node from which the record is to be removed
// - idx: The index of the record to be removed
// Description:
// This function removes a record from a leaf node in the B-tree. It deletes the record
// at the specified index and adjusts the remaining records to fill the gap. If necessary,
// it may perform a redistribution or merge operation to maintain the B-tree properties.
void removeFromLeaf (bTree* tree, bTreeNode *node, int idx) {
    for (int i=idx+1; i<node->noOfRecs; ++i){
	    node->children[i-1] = node->children[i];
    }
    node->noOfRecs--;
}
 
// Removes a record from a non-leaf node
// Parameters:
// - tree: The B-tree
// - node: The non-leaf node from which the record is to be removed
// - idx: The index of the record to be removed
void removeFromNonLeaf(bTree* tree, bTreeNode *node, int idx) {
 
    int k = node->children[idx]->codigoLivro;
    
    bTreeNode *child = malloc(sizeof(bTreeNode));
    bTreeNode *sibling = malloc(sizeof(bTreeNode));
    
    readFile(tree, child, node->children[idx]);
    readFile(tree, sibling, node->children[idx+1]); 
 
    
    
    if (child->noOfRecs >= t) {
        recordNode* pred = getPred(tree, node, idx);
        node->children[idx] = pred;
        removeNode(tree, child, pred->codigoLivro); 
    }
 
    
    else if  (sibling->noOfRecs >= t)
    {
        recordNode* succ = getSucc(tree, node, idx);
        node->children[idx] = succ;
        removeNode(tree, sibling, succ->codigoLivro); 
    }
 
        else {
        child = merge(tree, node, idx); 
        removeNode(tree, child, k);
	    return; 
    }
    
    writeFile(tree, child, child->pos);
    writeFile(tree, sibling, sibling->pos);

    free(child);
    free(sibling);
}

// Removes a node from the B-tree
// Parameters:
// - tree: The B-tree
// - node: The node to be removed
// - k: The key of the record to be removed
void removeNode(bTree* tree, bTreeNode* node, int k) {

    int idx = findKey(node, k);
    if (idx < node->noOfRecs && node->children[idx]->codigoLivro == k) {
        if (node->isLeaf){
	        removeFromLeaf(tree, node, idx);
        } else {
            removeFromNonLeaf(tree, node, idx);
        }
        
	    writeFile(tree, node, node->pos);
    }
    else {
       
        if (node->isLeaf) {
		    return;
       	}
 
        bool flag = idx==node->noOfRecs;
 

        bTreeNode *childAtPosi = malloc(sizeof(bTreeNode));
        readFile(tree, childAtPosi, node->children[idx]);

        if (childAtPosi->noOfRecs < t) {
            fill(tree, node, idx);
            readFile(tree, childAtPosi, node->children[idx]);
        }

        if (flag && idx > node->noOfRecs) {
            bTreeNode *sibling = malloc(sizeof(bTreeNode));
            readFile(tree, sibling, node->children[idx-1]);
            removeNode(tree, sibling, k);

            writeFile(tree, sibling, sibling->pos);
            free(sibling);
        }
        else {
            removeNode(tree, childAtPosi, k);
        }
        
        writeFile(tree, childAtPosi, childAtPosi->pos);
        free(childAtPosi);
    }
}

// Finds the predecessor of a key in the B-tree
// Parameters:
// - tree: The B-tree
// - node: The node from which the predecessor is to be found
// - idx: The index of the key in the node
// Returns:
// - The predecessor record of the specified key, or NULL if not found
recordNode* getPred(bTree* tree, bTreeNode *node, int idx) {
    bTreeNode *curr = malloc(sizeof(bTreeNode));
    readFile(tree, curr, node->children[idx]);

    while (!curr->isLeaf){
        readFile(tree, curr, curr->children[curr->noOfRecs]);
    }
        
    recordNode* result = curr->children[curr->noOfRecs-1];
    free(curr);
    return result;
}

// Finds the successor of a key in the B-tree
// Parameters:
// - tree: The B-tree
// - node: The node from which the successor is to be found
// - idx: The index of the key in the node
// Returns:
// - The successor record of the specified key, or NULL if not found
recordNode* getSucc(bTree* tree, bTreeNode *node, int idx) {
 
    bTreeNode *curr = malloc(sizeof(bTreeNode));
    readFile(tree, curr, node->children[idx+1]); 
    while (!curr->isLeaf){
        readFile(tree, curr, curr->children[0]);
    }
 
    
    recordNode* result = curr->children[0];
    free(curr);
    return result;
}
 
// Fills a child node with keys from its siblings or merges the child nodes
// Parameters:
// - tree: The B-tree
// - node: The parent node containing the child nodes
// - idx: The index of the child node to be filled or merged
void fill(bTree* tree, bTreeNode *node, int idx) {
    bTreeNode *cPrev = malloc(sizeof(bTreeNode));
    bTreeNode *cSucc = malloc(sizeof(bTreeNode));

    readFile(tree, cPrev, node->children[idx-1]);
    readFile(tree, cSucc, node->children[idx+1]);
    
    if (idx!=0 && cPrev->noOfRecs>=t) {
        borrowFromPrev(tree, node, idx);
    }
 

    else if (idx!=node->noOfRecs && cSucc->noOfRecs>=t) {
        borrowFromNext(tree, node, idx);
    }
 
    else {
        if (idx != node->noOfRecs)
            merge(tree, node, idx);
        else
            merge(tree, node, idx-1);
    }

    free(cPrev);
    free(cSucc);

    return;
}

// Borrows a key from the previous child node
// Parameters:
// - tree: The B-tree
// - node: The node from which the key is to be borrowed
// - idx: The index of the key in the node
// Description:
// This function borrows a key from the previous child node of a B-tree node. It updates
// the node by moving the rightmost key from the left child node to the current node and
// adjusting the pointers accordingly. This operation helps rebalance the B-tree after a deletion.
void borrowFromPrev(bTree* tree, bTreeNode *node, int idx) {
    bTreeNode *child = malloc(sizeof(bTreeNode));
    bTreeNode *sibling = malloc(sizeof(bTreeNode));
    
    readFile(tree, child, node->children[idx]);
    readFile(tree, sibling, node->children[idx-1]);
    

    for (int i=child->noOfRecs-1; i>=0; --i)
        child->children[i+1] = child->children[i];
 
    if (!child->isLeaf) {
        for(int i=child->noOfRecs; i>=0; --i)
            child->children[i+1] = child->children[i];
    }
 
    child->children[0] = node->children[idx-1];
 
    if (!node->isLeaf) {
        child->children[0] = sibling->children[sibling->noOfRecs];
        sibling->children[sibling->noOfRecs] = -1; 
    }
 
    node->children[idx-1] = sibling->children[sibling->noOfRecs-1];
 
    child->noOfRecs += 1;
    sibling->noOfRecs -= 1;
    
    writeFile(tree, node, node->pos);
    writeFile(tree, child, child->pos);
    writeFile(tree, sibling, sibling->pos);
    
    free(child);
    free(sibling);

    return;
}

// Borrows a key from the next child node
// Parameters:
// - tree: The B-tree
// - node: The node from which the key is to be borrowed
// - idx: The index of the key in the node
// Description:
// This function borrows a key from the next child node of a B-tree node. It updates the
// node by moving the leftmost key from the right child node to the current node and adjusting
// the pointers accordingly. This operation helps rebalance the B-tree after a deletion.
void borrowFromNext(bTree* tree, bTreeNode *node, int idx) {
 
    bTreeNode *child = malloc(sizeof(bTreeNode));
    bTreeNode *sibling = malloc(sizeof(bTreeNode));
    
    readFile(tree, child, node->children[idx]);
    readFile(tree, sibling, node->children[idx+1]);
    
    child->children[(child->noOfRecs)] = node->children[idx];
 
    if (!(child->isLeaf))
        child->children[(child->noOfRecs)+1] = sibling->children[0];
 
    node->children[idx] = sibling->children[0];
 
    for (int i=1; i<sibling->noOfRecs; ++i)
        sibling->children[i-1] = sibling->children[i];
 
    if (!sibling->isLeaf) {
        for(int i=1; i<=sibling->noOfRecs; ++i)
            sibling->children[i-1] = sibling->children[i];

        sibling->children[sibling->noOfRecs] = -1; //RESPOSTA:
    }
 
    child->noOfRecs += 1;
    sibling->noOfRecs -= 1;
    
    writeFile(tree, node, node->pos);
    writeFile(tree, child, child->pos);
    writeFile(tree, sibling, sibling->pos);

    free(child);
    free(sibling);

    return;
}

// Merges a child node with its sibling
// Parameters:
// - tree: The B-tree
// - node: The parent node containing the child nodes
// - idx: The index of the child node to be merged
// Returns:
// - The updated parent node after the merge operation
// Description:
// This function merges a child node with its sibling in a B-tree. It combines the keys and
// child pointers of the two nodes and updates the parent node accordingly. This operation
// helps maintain the B-tree properties by reducing the height of the tree and balancing
// the keys among the child nodes.
bTreeNode* merge(bTree* tree, bTreeNode *node, int idx) {

    bTreeNode *child = malloc(sizeof(bTreeNode));
    bTreeNode *sibling = malloc(sizeof(bTreeNode));
    
    readFile(tree, child, node->children[idx]);
    readFile(tree, sibling, node->children[idx+1]);
    
    child->children[t-1] = node->children[idx];
 
    for (int i=0; i<sibling->noOfRecs; ++i)
        child->children[i+t] = sibling->children[i];
 
    if (!child->isLeaf) {
        for(int i=0; i<=sibling->noOfRecs; ++i)
            child->children[i+t] = sibling->children[i];
    }

    for (int i=idx+1; i<node->noOfRecs; ++i)
        node->children[i-1] = node->children[i];
 
    for (int i=idx+2; i<=node->noOfRecs; ++i) 
        node->children[i-1] = node->children[i];
    node->children[node->noOfRecs] = -1; 
    child->noOfRecs += sibling->noOfRecs+1;
    node->noOfRecs--;
 
    if(node->noOfRecs == 0) {
        tree->root = node->children[0];
    }

    writeFile(tree, node, node->pos);
    writeFile(tree, child, child->pos);
    writeFile(tree, sibling, sibling->pos);

    free(sibling);

    return child;
}

bool removeFromTree(bTree* tree, int codigoLivro) {
    bTreeNode *root = malloc(sizeof(bTreeNode));
    readFile(tree, root, tree->root);

    bool found = search(tree, codigoLivro);
    if(found);
        removeNode(tree, root, codigoLivro); 

    free(root);
    return found;
}

void hardPrint(bTree* tree) {
    bTreeNode* lido = (bTreeNode*) malloc(sizeof(bTreeNode));
    for(int i = 0; i < tree->nextPos; i++) {
        fseek(tree->fp, i * sizeof(bTreeNode), SEEK_SET);
        fread(lido, sizeof(bTreeNode), 1, tree->fp);

        if(lido->isLeaf <= 1)
            dispNode(lido);
        else
            printf("ERRO: isLeaf = %i\n\n", lido->isLeaf);
    }

    free(lido);
}

void doublePrint(bTree* tree) {
    printf("=================");
    printf("\nTraverse\n");
    traverse(tree, tree->root);

    printf("=================");
    printf("\nHard print\n");
    hardPrint(tree);
}