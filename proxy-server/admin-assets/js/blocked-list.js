function showNotification(message, type = 'success') {
    const notification = document.getElementById('notification');
    const notificationText = document.getElementById('notificationText');
    
    notification.className = `notification ${type}`;
    notificationText.textContent = message;
    
    notification.classList.add('show');
    
    setTimeout(() => {
        notification.classList.remove('show');
    }, 3000);
}

function isValidIP(ip) {
    const ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
    if (!ipPattern.test(ip)) return false;
    
    const parts = ip.split('.').map(Number);
    return parts.every(num => num >= 0 && num <= 255);
}

async function loadBlacklist() {
    const tableBody = document.getElementById('tableBody');
    
    try {
        tableBody.innerHTML = `
            <tr>
                <td colspan="5" class="empty-state">
                    <div class="spinner"></div>
                    <h3>Загрузка данных...</h3>
                </td>
            </tr>
        `;
        
        const response = await fetch('/admin/api/blacklist');
        
        if (!response.ok) {
            throw new Error(`Ошибка загрузки: ${response.status}`);
        }
        
        const data = await response.json();
        
        if (data.length === 0) {
            tableBody.innerHTML = `
                <tr>
                    <td colspan="5" class="empty-state">
                        <i class="fas fa-user-check"></i>
                        <h3>Черный список пуст</h3>
                        <p>Нет заблокированных пользователей</p>
                    </td>
                </tr>
            `;
            return;
        }
        
        renderTable(data);
    } catch (error) {
        console.error('Ошибка при загрузке данных:', error);
        tableBody.innerHTML = `
            <tr>
                <td colspan="5" class="empty-state">
                    <i class="fas fa-exclamation-triangle"></i>
                    <h3>Ошибка загрузки</h3>
                    <p>${error.message || 'Не удалось загрузить данные'}</p>
                    <button id="retryBtn" class="btn" style="margin-top: 1rem; background: var(--primary); color: white;">
                        <i class="fas fa-redo"></i> Попробовать снова
                    </button>
                </td>
            </tr>
        `;
        
        document.getElementById('retryBtn')?.addEventListener('click', loadBlacklist);
    }
}

function renderTable(data) {
    const tableBody = document.getElementById('tableBody');
    tableBody.innerHTML = '';
    
    data.forEach(user => {
        const row = document.createElement('tr');
        
        let statusClass = 'blocked';
        let statusText = user.status;
        
        if (user.status.includes('Истекает')) {
            statusClass = 'expiring';
            statusText = `<span class="status-indicator ${statusClass}"></span>${user.status}`;
        } else {
            statusText = `<span class="status-indicator ${statusClass}"></span>${user.status}`;
        }
        
        row.innerHTML = `
            <td class="ip-address">${user.ip}</td>
            <td class="block-date">${user.block_date}</td>
            <td>${statusText}</td>
            <td class="reason">${user.reason}</td>
            <td class="actions-cell">
                <button class="btn unblock-btn" data-ip="${user.ip}">
                    <i class="fas fa-unlock"></i> Разблокировать
                </button>
            </td>
        `;
        
        tableBody.appendChild(row);
    });
    
    document.querySelectorAll('.unblock-btn').forEach(button => {
        button.addEventListener('click', async function() {
            const ip = this.dataset.ip;
            const row = this.closest('tr');
            
            this.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Обработка...';
            this.disabled = true;
            
            try {
                const response = await fetch(`/admin/api/blacklist/${encodeURIComponent(ip)}`, {
                    method: 'DELETE'
                });
                
                if (!response.ok) {
                    throw new Error(`Ошибка разблокировки: ${response.status}`);
                }
                
                row.style.transition = 'transform 0.3s ease, opacity 0.3s ease';
                row.style.transform = 'translateX(-100%)';
                row.style.opacity = '0';
                
                setTimeout(() => {
                    row.remove();
                    
                    if (document.querySelectorAll('#tableBody tr').length === 0) {
                        tableBody.innerHTML = `
                            <tr>
                                <td colspan="5" class="empty-state">
                                    <i class="fas fa-user-check"></i>
                                    <h3>Черный список пуст</h3>
                                    <p>Нет заблокированных пользователей</p>
                                </td>
                            </tr>
                        `;
                    }
                }, 300);
                
                showNotification(`IP-адрес ${ip} успешно разблокирован`, 'success');
            } catch (error) {
                console.error('Ошибка при разблокировке:', error);
                this.innerHTML = '<i class="fas fa-unlock"></i> Разблокировать';
                this.disabled = false;
                showNotification(`Ошибка разблокировки: ${error.message || 'Неизвестная ошибка'}`, 'error');
            }
        });
    });
}

async function blockUser(ip, reason) {
    const submitBtn = document.getElementById('submitBlock');
    const originalText = submitBtn.innerHTML;
    
    try {
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Блокировка...';
        submitBtn.disabled = true;
        
        const response = await fetch('/admin/api/blacklist', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ ip, reason })
        });
        
        if (!response.ok) {
            throw new Error(`Ошибка блокировки: ${response.status}`);
        }
        
        const newUser = await response.json();
        
        const tableBody = document.getElementById('tableBody');
        
        if (tableBody.querySelector('.empty-state')) {
            tableBody.innerHTML = '';
        }
        
        const newRow = document.createElement('tr');
        newRow.innerHTML = `
            <td class="ip-address">${newUser.ip}</td>
            <td class="block-date">${newUser.block_date}</td>
            <td><span class="status-indicator blocked"></span>Активна</td>
            <td class="reason">${newUser.reason}</td>
            <td class="actions-cell">
                <button class="btn unblock-btn" data-ip="${newUser.ip}">
                    <i class="fas fa-unlock"></i> Разблокировать
                </button>
            </td>
        `;
        
        tableBody.prepend(newRow);
        
        newRow.querySelector('.unblock-btn').addEventListener('click', async function() {
            const ip = this.dataset.ip;
            const row = this.closest('tr');
            
            this.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Обработка...';
            this.disabled = true;
            
            try {
                const response = await fetch(`/admin/api/blacklist/${encodeURIComponent(ip)}`, {
                    method: 'DELETE'
                });
                
                if (!response.ok) {
                    throw new Error(`Ошибка разблокировки: ${response.status}`);
                }
                
                row.style.transition = 'transform 0.3s ease, opacity 0.3s ease';
                row.style.transform = 'translateX(-100%)';
                row.style.opacity = '0';
                
                setTimeout(() => {
                    row.remove();
                    
                    if (document.querySelectorAll('#tableBody tr').length === 0) {
                        tableBody.innerHTML = `
                            <tr>
                                <td colspan="5" class="empty-state">
                                    <i class="fas fa-user-check"></i>
                                    <h3>Черный список пуст</h3>
                                    <p>Нет заблокированных пользователей</p>
                                </td>
                            </tr>
                        `;
                    }
                }, 300);
                
                showNotification(`IP-адрес ${ip} успешно разблокирован`, 'success');
            } catch (error) {
                console.error('Ошибка при разблокировке:', error);
                this.innerHTML = '<i class="fas fa-unlock"></i> Разблокировать';
                this.disabled = false;
                showNotification(`Ошибка разблокировки: ${error.message || 'Неизвестная ошибка'}`, 'error');
            }
        });
        
        document.getElementById('blockModal').style.display = 'none';
        document.getElementById('blockForm').reset();
        
        showNotification(`IP-адрес ${ip} успешно заблокирован`, 'success');
    } catch (error) {
        console.error('Ошибка при блокировке:', error);
        showNotification(`Ошибка блокировки: ${error.message || 'Неизвестная ошибка'}`, 'error');
    } finally {
        submitBtn.innerHTML = originalText;
        submitBtn.disabled = false;
    }
}

document.getElementById('blockUserBtn').addEventListener('click', function() {
    document.getElementById('blockModal').style.display = 'flex';
    document.getElementById('ipInput').focus();
});

document.getElementById('closeModal').addEventListener('click', function() {
    document.getElementById('blockModal').style.display = 'none';
    document.getElementById('blockForm').reset();
    document.getElementById('ipError').style.display = 'none';
    document.getElementById('reasonError').style.display = 'none';
});

document.getElementById('cancelBlock').addEventListener('click', function() {
    document.getElementById('blockModal').style.display = 'none';
    document.getElementById('blockForm').reset();
    document.getElementById('ipError').style.display = 'none';
    document.getElementById('reasonError').style.display = 'none';
});

document.getElementById('blockForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ip = document.getElementById('ipInput').value.trim();
    const reason = document.getElementById('reasonInput').value.trim();
    let isValid = true;
    
    if (!isValidIP(ip)) {
        document.getElementById('ipError').style.display = 'block';
        isValid = false;
    } else {
        document.getElementById('ipError').style.display = 'none';
    }
    
    if (reason.length < 5) {
        document.getElementById('reasonError').style.display = 'block';
        isValid = false;
    } else {
        document.getElementById('reasonError').style.display = 'none';
    }
    
    if (isValid) {
        blockUser(ip, reason);
    }
});

document.addEventListener('DOMContentLoaded', loadBlacklist);