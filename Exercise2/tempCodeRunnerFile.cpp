

        ++degree[u];
        if (degree[u] > 2) {
            return false;
        }
        ++degree[v];