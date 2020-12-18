import os
import pickle
import subprocess
from argparse import ArgumentParser

import numpy as np
import scipy.sparse as ssp

try:
    import DPGS
except ImportError as e:
    subprocess.run(["python", "setup.py", "build_ext", "--inplace"])
    import DPGS


parser = ArgumentParser()
parser.add_argument('dataset', type=str, help='Dataset name')
parser.add_argument('adj', type=str,
                    help='Path of adjacency matrix(.npz sparse matrix format)')
parser.add_argument('--feature', type=str, default='',
                    help='Path of feature file(.npy format), ignore it if no features are provided')
parser.add_argument('--turn', type=int, default=30,
                    help='Iteration turn(default 30)')
parser.add_argument('--output_dir', type=str,
                    default='./output', help='Output directory')
parser.add_argument('--b', type=int, default=8, help='Maximum band of LSH')
parser.add_argument('--seed', type=int, default=42, help='RNG seed')
parser.add_argument('--debug', action='store_true',
                    default=False, help='Debug flag')

args = parser.parse_args()

if __name__ == '__main__':
    sm = ssp.load_npz(args.adj)
    N = sm.shape[0]
    sm = sm.tocoo()
    row = sm.row.astype(np.uint64)
    col = sm.col.astype(np.uint64)

    graph = DPGS.from_row_col(sm.shape[0], row, col)
    model = DPGS.DPGS(args.dataset, graph, args.b, args.seed, args.debug)
    model.run(args.turn)
    nodes_dict = model.getNodesDict()
    nodes_dict = dict((i, sn)
                      for (i, sn) in enumerate(nodes_dict) if len(sn) > 0)

    if not os.path.exists(f'./output/{args.dataset}'):
        os.mkdir(f'./output/{args.dataset}')
    pickle.dump(nodes_dict, open(
        f'./output/{args.dataset}/nodes_dict.pkl', 'wb'))
