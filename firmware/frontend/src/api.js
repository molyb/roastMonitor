export async function fetchLog(since) {
    let url = '/log';
    if (since !== undefined && since !== null) {
        url += `?since=${since}`;
    }
    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`HTTP error ${response.status}`);
    }
    return response.json();
}

export async function fetchTc() {
    const response = await fetch('/tc', { cache: 'no-store' });
    if (!response.ok) {
        throw new Error(`HTTP error ${response.status}`);
    }
    return response.json();
}
