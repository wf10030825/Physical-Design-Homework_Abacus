#include "abacus.h"
using namespace std;
Abacus::Abacus()
{
    // n = module_map.size();
}

Abacus::~Abacus(){
    // 清理所有動態配置的 Sub-Rows
    for (auto* row : ROWS) {
        for (auto* sub : row->sub_rows) {
            delete sub;
        }
        row->sub_rows.clear();
    }
}

double AlignToSite(double x, ROW* row){
    if (row->Sitespacing <= 0)
        return x; // 防呆
    if (ROWS.empty())
        return x;

    // [FIX 4] Use ROWS[0]->lb.x as global origin to ensure alignment with original rows
    double origin = ROWS[0]->lb.x;
    double sites = round((x - origin) / row->Sitespacing);
    double aligned_x = origin + sites * row->Sitespacing;

    return aligned_x;
}

void Abacus::algo(){

    cout << "Start Abacus Legalization..." << endl;

    cout << "Total modules: " << module_map.size() << endl;
    vector<MODULE*> module_vector;
    module_vector.reserve(module_map.size());
    for (const auto& i : module_map)
    {
        if (i.second->fixed)
            continue;
        module_vector.push_back(i.second);
    }
    cout << "Total movable modules: " << module_vector.size() << endl;

    // 根據 x 座標排序
    sort(module_vector.begin(), module_vector.end(), [](MODULE* a, MODULE* b)
        { return a->lb.x < b->lb.x; });


    int window_expansion = 3;
    int consecutive_success = 0;  // 連續成功次數
    const int shrink_threshold = 5;  // 連續成功多少次後縮小 window
    const int min_window = 3;  // window 最小值

    // 工作緩衝重複使用以降低分配成本
    vector<ROW*> total_subrow;
    vector<pair<MODULE*, double>> original_positions;

    int i = 0;
    while(i < module_vector.size()){

        MODULE* m = module_vector[i];
        double Cbest = DBL_MAX;
        double cost = 0;
        int best_row_index = -1; // best row

        int global_placement_row_index = (m->lb.y - ROWS[0]->lb.y) / ROWS[0]->W_H.y;

        int index = global_placement_row_index;
        if (index < 0)
            index = 0;  // align to first row
        if (index >= (int)ROWS.size())
            index = ROWS.size() - 1;    // align to last row

        // search for start (0~lower bound) and end(upper bound~top)
        int start_search = max(0, index - window_expansion);
        int end_search = min((int)ROWS.size() - 1, index + window_expansion);

        // collect subrows
        total_subrow.clear();
        total_subrow.reserve(min((end_search - start_search + 1) * 20, (int)ROWS.size() * 20));
        for (int i = start_search; i <= end_search; i++)    // search for the windows
        {
            const auto& sub_rows = ROWS[i]->sub_rows;
            total_subrow.insert(total_subrow.end(), sub_rows.begin(), sub_rows.end());
        }

        // 開始試算
        const size_t total_subrow_size = total_subrow.size();
        for (size_t i = 0; i < total_subrow_size; i++)
        {
            ROW* target_row = total_subrow[i];

            // 備份
            original_positions.clear();
            original_positions.reserve(target_row->modules.size());
            for (auto* mod : target_row->modules)
            {
                original_positions.push_back(make_pair(mod, mod->lb.x));
            }

            // 記錄 m 的原始位置
            double m_original_x = m->lb.x;
            double m_original_y = m->lb.y;

            // 放入試算
            target_row->modules.push_back(m);
            m->lb.y = target_row->lb.y;

            PlaceRow(target_row); // 排列並防重疊

            // 檢查是否超出 subrow 邊界
            bool is_overflow = false;
            const double limit = target_row->lb.x + target_row->W_H.x;
            if (!target_row->modules.empty())
            {
                const MODULE* last = target_row->modules.back();
                if (last->lb.x + last->W_H.x > limit + 0.001)
                {
                    is_overflow = true;
                }
            }

            if (!is_overflow)
            {
                // 只有沒爆掉才計算正常 Cost
                cost = 0;
                // 計算舊 Cells 位移
                for (const auto& record : original_positions)
                {
                    const MODULE* mod = record.first;
                    const double old_x = record.second;
                    cost += abs(mod->lb.x - old_x);
                }
                // 計算 m 位移
                const double m_dx = abs(m->lb.x - m_original_x);
                const double m_dy = abs(m->lb.y - m_original_y);
                cost += (m_dx + m_dy);

                if (cost < Cbest)
                {
                    Cbest = cost;
                    best_row_index = i;
                }
            }
            // 恢復狀態
            // 1. 移除 m
            auto it = find(target_row->modules.begin(), target_row->modules.end(), m);
            if (it != target_row->modules.end())
            {
                target_row->modules.erase(it);
            }
            // 2. 還原舊座標
            for (auto& record : original_positions)
            {
                record.first->lb.x = record.second;
            }
            // 3. 還原 m
            m->lb.x = m_original_x;
            m->lb.y = m_original_y;
        }

        // 5. 將 m 放入最終決定的最佳列

        if (best_row_index != -1)
        {
            ROW* best_subrow = total_subrow[best_row_index];
            best_subrow->modules.push_back(m);
            m->lb.y = best_subrow->lb.y;
            PlaceRow(best_subrow);
            
            // 連續成功，考慮縮小 window
            consecutive_success++;
            if (consecutive_success >= shrink_threshold && window_expansion > min_window)
            {
                window_expansion -= 2;
                if (window_expansion < min_window)
                    window_expansion = min_window;
                consecutive_success = 0;  // 重置計數
            }
            i++;
        }
        else
        {
            // cout << "Placement can't be found! Should expand search range." << endl;
            window_expansion += 2;
            consecutive_success = 0;  // 失敗時重置連續成功計數
        }
    }
}
void Abacus::Collapse(Cluster& C, Cluster& C_before, vector<Cluster>& clusters, double row_min, double row_max)
{
    C.xc = C.qc / C.ec; // x = q/e
    if (C.xc < row_min)
        C.xc = row_min; // left boundary
    if (C.xc + C.wc > row_max) // right boundary
        C.xc = row_max - C.wc;
    if (C_before.xc + C_before.wc > C.xc) // overlap check
    {
        // merge C_before and C
        C_before.end_index = C.end_index;
        // C_before.modules.insert(C_before.modules.end(), C.modules.begin(), C.modules.end());
        //  AddCluster(c-1,i)
        C_before.n_last = C.n_last;
        C_before.qc = C_before.qc + C.qc - C_before.wc * C.ec;
        C_before.ec = C_before.ec + C.ec;
        C_before.wc = C_before.wc + C.wc;
        // Collapse(c-1)
        clusters.pop_back(); // Remove last Cluster

        if (clusters.size() > 1)
        {
            Collapse(clusters.back(), clusters[clusters.size() - 2], clusters, row_min, row_max);
        }
    }
}

void Abacus::PlaceRow(ROW* row)
{
    auto cmp_module_x = [](MODULE* a, MODULE* b)    // sort by x
        { return a->lb.x < b->lb.x; };
    if (!is_sorted(row->modules.begin(), row->modules.end(), cmp_module_x))
    {
        sort(row->modules.begin(), row->modules.end(), cmp_module_x);
    }

    // int c = 0;
    // Cluster C;
    vector<Cluster> clusters;
    clusters.reserve(row->modules.size());
    double row_min = row->lb.x;
    double row_max = row->lb.x + row->W_H.x;

    // Align row_min UP to site grid
    if (row->Sitespacing > 0 && !ROWS.empty())
    {
        double spacing = row->Sitespacing;
        double origin = ROWS[0]->lb.x;
        double num = ceil((row_min - origin) / spacing);
        row_min = origin + num * spacing;
    }

    const size_t modules_size = row->modules.size();
    for (size_t i = 0; i < modules_size; i++)
    {
        // C = clusters[sizeof(clusters) - 1];
        // NO OVERLAP
        MODULE* m = row->modules[i];
        if (clusters.empty() || clusters.back().xc + clusters.back().wc <= m->lb.x)
        {
            Cluster new_Cluster;
            new_Cluster.start_index = i;
            new_Cluster.end_index = i;
            // new_Cluster.modules.push_back(row->modules[i]);

            new_Cluster.xc = m->lb.x;
            new_Cluster.n_first = i;

            // AddCell(c,i)
            new_Cluster.n_last = i;
            new_Cluster.qc = m->lb.x;
            new_Cluster.wc = m->W_H.x;
            new_Cluster.ec = 1;
            // store new_Cluster
            clusters.push_back(new_Cluster);
        }
        else
        {
            Cluster& C = clusters.back();
            // AddCell(c,i)
            C.end_index = i;
            C.n_last = i;
            C.ec = C.ec + 1;
            C.qc = C.qc + m->lb.x - C.wc;
            C.wc = C.wc + m->W_H.x;
            // cout << "cluste size before collapse: " << clusters.size() << endl;
            if (clusters.size() > 1)
            {
                Collapse(clusters.back(), clusters[clusters.size() - 2], clusters, row_min, row_max);
            }
            // Collapse(C, clusters[clusters.size() - 2], clusters);
        }
    }
    // avoid overlap when writing back positions
    double last_placed_x = row_min;
    for (const auto& cl : clusters)
    {
        double x = cl.xc;
        if (x < row_min)
            x = row_min;
        if (x + cl.wc > row_max)
            x = row_max - cl.wc;

        // Align to Site Grid
        double aligned_start = AlignToSite(x, row);

        // Prevent overlap: ensure not overlapping with previous Cluster
        if (aligned_start < last_placed_x)
        {
            aligned_start = AlignToSite(last_placed_x, row);
        }

        // Prevent overflow: ensure not exceeding row right boundary
        if (aligned_start + cl.wc > row_max)
        {
            aligned_start = row_max - cl.wc;
            // Ensure alignment
            aligned_start = AlignToSite(aligned_start, row);
            // If still overlapping after alignment, prioritize no overlap
            if (aligned_start < last_placed_x)
            {
                aligned_start = last_placed_x;
            }
        }
        // [步驟 C] 寫回 Cluster 內的所有 Cells
        double current_x = aligned_start;
        for (int i = cl.start_index; i <= cl.end_index; i++)
        {
            row->modules[i]->lb.x = current_x;
            current_x += row->modules[i]->W_H.x;
        }
        last_placed_x = current_x;
    }
}

void Abacus::slice_row(){
    // 1. 收集所有的 Fixed Modules (障礙物)
    cout << "Start Slice Row..." << endl;
    
    // 檢查 ROWS 是否為空
    if (ROWS.empty()) {
        cout << "ERROR: ROWS is empty! Cannot slice rows." << endl;
        return;
    }
    cout << "ROWS size: " << ROWS.size() << endl;
    
    vector<MODULE*> obstacles;
    obstacles.reserve(module_map.size() / 4);
    for (const auto& item : module_map)
    {
        if (item.second == nullptr) {
            cout << "WARNING: null MODULE pointer found in module_map" << endl;
            continue;
        }
        if (item.second->fixed)
        {
            obstacles.push_back(item.second);   // fixed module
        }
    }
    cout << "Fixed obstacles count: " << obstacles.size() << endl;

    // 2. 對每一條 Row 進行獨立處理
    int row_count = 0;
    for (auto* row : ROWS)
    {
        if (row == nullptr) {
            cout << "ERROR: null ROW pointer at index " << row_count << endl;
            row_count++;
            continue;
        }
        
        // 先清除舊的 Sub-Rows (以防重複呼叫)
        for (auto* sub : row->sub_rows)
            delete sub;
        row->sub_rows.clear();

        // 用來暫存這條 Row 上的所有「被阻擋區間」
        vector<pair<double, double>> blocked_intervals; // save all obstacle by each row
        blocked_intervals.reserve(obstacles.size());

        const double row_y_min = row->lb.y;
        const double row_y_max = row->lb.y + row->W_H.y;
        // 使用極小值防止邊界誤判 (Off-by-one fix)
        const double epsilon = 1e-4;

        for (auto it = obstacles.begin(); it != obstacles.end(); )
        {
            auto* obs = *it;
            // 檢查 Y 軸是否重疊
            if (obs->lb.y < row_y_max - epsilon && (obs->lb.y + obs->W_H.y) > row_y_min + epsilon)
            {
                double start_x = max(chip_lb.x, obs->lb.x);
                double end_x = min(chip_ur.x, obs->lb.x + obs->W_H.x);

                if (start_x < end_x)
                {
                    blocked_intervals.push_back({ start_x, end_x });
                }

                // 只有當障礙物完全在這個 Row 範圍內（不跨 Row）才刪除
                // 或者：如果這個 Row 是障礙物覆蓋的最後一個 Row，才刪除
                if (obs->lb.y + obs->W_H.y <= row_y_max + epsilon)
                {
                    it = obstacles.erase(it);
                }
                else
                {
                    ++it;  // 跨越多 Row，不刪除，繼續下一個
                }
            }
            else
            {
                ++it;  // Y 軸不重疊，繼續下一個
            }
        }

        // 3. Interval Merging
        sort(blocked_intervals.begin(), blocked_intervals.end());

        vector<pair<double, double>> merged;
        merged.reserve(blocked_intervals.size());
        for (const auto& interval : blocked_intervals)
        {
            if (merged.empty() || interval.first > merged.back().second)
            {
                // 如果 merged 為空，或是當前區間跟上一個區間沒重疊 -> 直接加入
                merged.push_back(interval);
            }
            else
            {
                // 多個區間重疊或相鄰，直接看成一個大的
                merged.back().second = max(merged.back().second, interval.second);
            }
        }

        // 4. 切割出 Sub-Rows (利用合併後的阻擋區間)
        const double row_lb_x = row->lb.x;
        const double row_end_x = row->lb.x + row->W_H.x;
        double current_x = row_lb_x;

        auto createSubRow = [&](double start_x, double end_x) {
            if (end_x <= start_x + epsilon)
                return;

            ROW* sub = new ROW();
            sub->lb.y = row->lb.y;
            sub->W_H.y = row->W_H.y;
            sub->lb.x = start_x;
            sub->W_H.x = end_x - start_x;
            sub->Sitespacing = row->Sitespacing;

            if (sub->Sitespacing > 1e-6)
            {
                double rel_x = sub->lb.x - chip_lb.x;
                double site_offset = ceil(rel_x / sub->Sitespacing) * sub->Sitespacing - rel_x;
                sub->lb.x += site_offset;
                sub->W_H.x -= site_offset;
                sub->NumSites = floor(sub->W_H.x / sub->Sitespacing);
            }
            else
            {
                sub->NumSites = 0;
            }

            if (sub->W_H.x > 1e-6)
            {
                row->sub_rows.push_back(sub);
            }
            else
            {
                delete sub;
            }
        };

        for (const auto& block : merged)
        {
            createSubRow(current_x, block.first);
            current_x = max(current_x, block.second);
        }

        // 5. 處理最後一段 (最後一個障礙物到 Row 右邊界)
        createSubRow(current_x, row_end_x);
    }
}