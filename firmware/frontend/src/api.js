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
