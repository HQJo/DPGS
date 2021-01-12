import os
import pickle

import numpy as np
import scipy.sparse as ssp


def construct_P(degs: np.array, nodes_dict: dict, dataset: str, ratio: float):
    print(f"N: {len(degs)}")
    print(f"n: {len(nodes_dict)}")
    N = len(degs)
    dir_ = f"output/{dataset}_{ratio:.1f}"
    np.save(os.path.join(dir_, 'degrees.npy'), degs)

    rows, cols, datas = [], [], []
    for i, nodes in enumerate(nodes_dict.values()):
        assert len(nodes) != 0
        D = sum(degs[n_] for n_ in nodes)
        nodes = list(nodes)
        rows.extend([i] * len(nodes))
        cols.extend(nodes)
        if D == 0:
            datas.extend([1.0 / len(nodes)] * len(nodes))
        else:
            datas.extend((degs[n_] / D for n_ in nodes))
    n = len(nodes_dict)
    P = ssp.coo_matrix((datas, (rows, cols)), shape=(n, N))
    P_ = ssp.coo_matrix(([1] * len(rows), (cols, rows)), shape=(N, n))
    ssp.save_npz(os.path.join(dir_, 'P.npz'), P)
    ssp.save_npz(os.path.join(dir_, 'P_.npz'), P_)
    return P, P_


def post_process(adj: ssp.spmatrix, nodes_dict: dict, dataset: str, ratio: float):
    dir_ = f"output/{dataset}_{ratio:.1f}"
    if not os.path.exists(dir_):
        os.mkdir(dir_)
    path = os.path.join(dir_, 'nodes_dict.pkl')
    pickle.dump(nodes_dict, open(path, 'wb'))

    degs = np.array(adj.sum(axis=1)).squeeze()
    P, P_ = construct_P(degs, nodes_dict, dataset, ratio)
    adj_s = P_.T @ adj @ P_
    ssp.save_npz(os.path.join(dir_, 'A_s.npz'), adj_s)
