MastersThesis
=============

Work towards my masters thesis

Dr. Duan,

I was considering continuing my work on OCR for my masters thesis. I was wondering if you thought that this would be a good topic to choose, and if so, if would you be interested in being my advisor? Below is the outline of how I expect the complete OCR process to work.

Thanks,
Chris

Research
- Research English/Roman character normalization techniques
Survey English/Roman font categorization
Continue researching the Eigen-face technique and verify my implementation
Research seam carving algorithms
Continue researching k-Nearest Neighbor
Determine, mathematically, why cosine similarity is a better metric than Euclidean distance
Research English character frequencies/probabilities
Survey spell-checking algorithms

Training
Devise better character normalization heuristics (to transform all characters into a 16 by 16 pixel space)
Generate training images by font category (serif, old style serif, modern serif, slab serif, sans serif, black letter/gothic, typewriter, moveable type)
Generate training images for 96 printable ASCII characters (and 32 ligatures, as needed)
Generate mean images for each character in each category
Use PCA from SVD to generate Eigen-image-space
Project mean images onto Eigen-image-space to generate weights
Store eigen-image-space and weights for testing

Testing
Use a dynamic programming algorithm for seam carving characters from the text image
Use improved character normalization heuristics to standardize extracted characters
Change k-Nearest Neighbor to use k > 1 (now that more than one instance of each character will be available)
Experiment with incorporating English letter probabilities into the k-NN algorithm
Experiment with spell checking the recognized text (possibly using Rabin-Karp)
Experiment with using a dynamic programming algorithm to correct misspelled words

Results
A working command line program written in C which will perform fast and reliable OCR
A 40-60 page thesis paper
