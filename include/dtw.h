#ifndef _DTW_H_
#define _DTW_H_
// short DTWDistance(const short* a, const short* b) {
//     for (int i = 0; i < N; ++i) {
//         for (int j = 0; j < N; ++j) {
//             short cost = absl(*(a+i), *(b+j));
//             DTW[i+1][j+1] = cost + min3(DTW[i][j+1], DTW[i+1][j], DTW[i][j]);
//         }
//         short res = DTW[N][N];
//         DTW[N][N] = {32767};
//         DTW[0][0] = 0;
//         return res;
//     }
// }
#endif