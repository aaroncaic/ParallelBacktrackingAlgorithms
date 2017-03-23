import java.util.*;


public class mqds {

	static int qRecord;     // Min number of queens to dominate board
	static int[][] sol;			// Final solution board
	static int n;						// Board dimensions

	/*
	 * Copy contents from src to dest.
	 */
	public static void copyArray(int[][] src, int[][] dest) {
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				dest[i][j] = src[i][j];
			}
		}
	}

	/*
	 * Place a queen at (i,j) on board and update queen's
	 * attacking positions on board. Keep track and return number of
	 * number of spots on board that does not contain a queen
	 * and isn't attacked.
	 */
	public static int putQueens(int[][] b, int i, int j, int numQ, int numZeros) {
		if(b[i][j] > 0) {
			System.out.println("Something went wrong");
			return -1;
		}

		int newNumZeros = numZeros;

		if(b[i][j] == 0) { newNumZeros--;}
		b[i][j] = numQ + 1;

		// Attack entire column
		for(int k = i + 1; k < n; k++) {
			if(b[k][j] == 0) { newNumZeros--; }
			if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }
		}
		for(int k = i - 1; k >= 0; k--) {
			if(b[k][j] == 0) { newNumZeros--; }
			if(b[k][j] <= 0) { b[k][j] = -1 * (numQ + 1); }

		}

		// Attack entire row
		for(int k = j + 1; k < n; k++) {
			if(b[i][k] == 0) { newNumZeros--; }
			if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
		}
		for(int k = j - 1; k >= 0; k--) {
			if(b[i][k] == 0) { newNumZeros--; }
			if(b[i][k] <= 0) { b[i][k] = -1 * (numQ + 1); }
		}

		// Attack all diagonals
		int k = i + 1;
		int l = j + 1;
		while(k < n && l < n) {
			if(b[k][l] == 0) { newNumZeros--; }
			if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
			k++;
			l++;
		}
		k = i - 1;
		l = j - 1;
		while(k >= 0 && l >= 0) {
			if(b[k][l] == 0) { newNumZeros--; }
			if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
			k--;
			l--;
		}
		k = i + 1;
		l = j - 1;
		while(k < n && l >= 0) {
			if(b[k][l] == 0) { newNumZeros--; }
			if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
			k++;
			l--;
		}
		k = i - 1;
		l = j + 1;
		while(k >= 0 && l < n) {
			if(b[k][l] == 0) { newNumZeros--; }
			if(b[k][l] <= 0) { b[k][l] = -1 * (numQ + 1); }
			k--;
			l++;
		}

		return newNumZeros;
	}

	public static void QDBT(int[][] b, int i, int j, int numQ, int numZeros) {
		// Exeeded min number of queens, return
		if(numQ >= qRecord) {
			return;
		}

		// Place queen on (i,j) and update number of non-attacked spots
		int [][]newB = new int[n][n];
		copyArray(b, newB);
		int newNumZeros = putQueens(newB, i, j, numQ, numZeros);
		// for(int a = 0; a < n; a++) {
		// 	String row = "";
		// 	for(int c = 0; c < n; c++) {
		// 		if(newB[a][c] < 0) {
		// 			row = row + "[" + newB[a][c] + "]";
		// 		} else {
		// 			row = row + "[ " + newB[a][c] + "]";
		// 		}
		// 	}
		// 	System.out.println(row);
		// }
		// System.out.println("Number of zeros: " + newNumZeros + "\n");
		// Found a new min solution, update solution.
		// Return since any solution other solutions
		// on this branch is not minimum.
		if(newNumZeros <= 0 && numQ < qRecord) {
			qRecord = numQ;
			sol = newB;
			return;
		}

		// Compute position of next queen
		int newI = -1;
		int newJ = -1;
		if(j < n - 1) {
			newJ = j + 1;
			newI = i;
		} else if (i < n - 1){
			newI = i + 1;
			newJ = 0;
		} else {
			return;
		}

		// Recursively branch out
		QDBT(newB, newI, newJ, numQ+1, newNumZeros);

		// Backtrack
		QDBT(b, newI, newJ, numQ, numZeros);
	}


	public static void main(String[] args) {

		// Get n from command line arguments
		if (args.length > 0) {
		    try {
		        n = Integer.parseInt(args[0]);
		    } catch (NumberFormatException e) {
		        System.err.println("Argument" + args[0] + " must be an integer.");
		        System.exit(1);
		    }
		} else {
			System.exit(1);
		}

		// Set minimum number of queens needed to dominate board
		qRecord = Integer.MAX_VALUE;

		// Create initial board
		int[][] b = new int[n][n];
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				b[i][j] = 0;
			}
		}

		// Number of spots on board that are not attacked
		int numZeros = n*n;

		// Execute algorithm and time it
		long startTime = System.nanoTime();
		QDBT(b, 0, 0, 0, numZeros);
		long endTime = System.nanoTime();

		// Get execution time in ms
		long duration = ((endTime - startTime) / 1000000);

		// Output solution
		System.out.println("Solution:");
		for(int i = 0; i < n; i++) {
			String row = "";
			for(int j = 0; j < n; j++) {
				if(sol[i][j] <= 0) {
					row = row + "[" + sol[i][j] + "]";
				} else {
					row = row + "[ " + sol[i][j] + "]";
				}
			}
			System.out.println(row);
		}

		System.out.println("Execution Time: " + duration + " ms");
	}

}
